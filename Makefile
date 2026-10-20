SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.SILENT:
.DEFAULT_GOAL := help

# Toolchain
CC := gcc
CLANG_FORMAT ?= clang-format-22

# Directories
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
COVERAGE_BUILD_DIR := $(BUILD_DIR)/coverage
COVERAGE_REPORT_DIR := coverage
CONFIG_DIR := config
OUTPUT_DIR := output
TEST_DIR := tests
SUPPORT_DIR := $(TEST_DIR)/support
FORMAT_SOURCES := $(wildcard src/*.c include/*.h tests/*.c tests/support/*.c tests/support/*.h)

# Compiler options
CFLAGS := -O3 -std=c11 -Wall -Wextra -D_DEFAULT_SOURCE -Iinclude -Isrc -I$(SUPPORT_DIR) -fno-builtin-sincos
DEPFLAGS := -MMD -MP
LDLIBS := -lm

# Sources and objects
CORE_SOURCES := $(filter-out src/simulate.c src/plot_main.c,$(wildcard src/*.c))
COVERAGE_SOURCES := $(CORE_SOURCES) src/simulate.c src/plot_main.c
COVERAGE_REPORT_FILES := $(addsuffix .gcov,$(notdir $(COVERAGE_SOURCES)))
CORE_OBJECTS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(CORE_SOURCES))
SUPPORT_SOURCES := $(wildcard $(SUPPORT_DIR)/*.c)
SUPPORT_OBJECTS := $(patsubst $(SUPPORT_DIR)/%.c,$(OBJ_DIR)/support_%.o,$(SUPPORT_SOURCES))
TEST_SOURCES := $(wildcard $(TEST_DIR)/test_*.c)
TEST_OBJECTS := $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.o,$(TEST_SOURCES))
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SOURCES))
APP_BINS := $(BUILD_DIR)/simulate $(BUILD_DIR)/plot_main
ENTRY_OBJECTS := $(OBJ_DIR)/simulate_entry.o $(OBJ_DIR)/plot_main_entry.o
DEPFILES := $(CORE_OBJECTS:.o=.d) $(SUPPORT_OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d) \
	$(ENTRY_OBJECTS:.o=.d) $(OBJ_DIR)/simulate_app.d $(OBJ_DIR)/plot_main_app.d

-include $(DEPFILES)

# User-configurable workflow parameters
CONFIG ?= $(CONFIG_DIR)/config.dat
NAME ?= default
N_RUNS ?= 20
GEOMETRY ?= sphere
DISTRIBUTION ?=
PNG ?=
PLOT_TYPE ?= both

.PHONY: all help \
	build test coverage format format-check \
	run run-one plot run-many \
	sweep report full quality clean .replot-sweep

all: build test

help:
	@printf '%s\n' \
	  'Build and tests:' \
	  '  make build           Build application and tests' \
	  '  make test            Build and run every test suite' \
	  '  make coverage        Run tests with gcov and write coverage/*.gcov' \
	  '  make format          Apply clang-format-22 before pushing' \
	  '  make format-check    Verify formatting without modifying files' \
	  'Run and plot:' \
	  '  make run             Run the default simulation and plot' \
	  '  make run-one CONFIG=... NAME=... Run one named experiment' \
	  '  make plot DISTRIBUTION=... [CONFIG=...] [PNG=...] Plot existing distribution CSV' \
	  '    PLOT_TYPE=distribution|distribution-linear|distribution-mu|distribution-mu-linear|phase|both (default both)' \
	  '  make run-many N_RUNS=20 Check seed stability' \
	  'Sweeps and reports:' \
	  '  make sweep [GEOMETRY=sphere|cube] Run the parameter grid (default: sphere)' \
	  '  make report          Regenerate sweep plots and the HTML report' \
	  '  make full            Build, test, sweep, and report' \
	  'Quality and cleanup:' \
	  '  make quality         Check formatting and run static analysis' \
	  '  make clean           Remove build/output and generated configs'

# Build and tests
build: $(APP_BINS) $(TEST_BINS)

$(OBJ_DIR):
	mkdir -p "$@"

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/support_%.o: $(SUPPORT_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/test_%.o: $(TEST_DIR)/test_%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/simulate_entry.o: src/simulate.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -Dmain=simulate_entrypoint -c "$<" -o "$@"

$(OBJ_DIR)/plot_main_entry.o: src/plot_main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -Dmain=plot_main_entrypoint -c "$<" -o "$@"

$(BUILD_DIR)/simulate: src/simulate.c $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $(DEPFLAGS) -MF "$(OBJ_DIR)/simulate_app.d" -MT "$@" $(CORE_OBJECTS) "$<" $(LDLIBS) -o "$@"

$(BUILD_DIR)/plot_main: src/plot_main.c $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $(DEPFLAGS) -MF "$(OBJ_DIR)/plot_main_app.d" -MT "$@" $(CORE_OBJECTS) "$<" $(LDLIBS) -o "$@"

$(BUILD_DIR)/test_%: $(OBJ_DIR)/test_%.o $(CORE_OBJECTS) $(SUPPORT_OBJECTS) $(ENTRY_OBJECTS)
	$(CC) $(CORE_OBJECTS) $(SUPPORT_OBJECTS) $(ENTRY_OBJECTS) "$<" $(LDLIBS) -o "$@"

test: $(TEST_BINS)
	set -e
	for test_bin in $(TEST_BINS); do
	  "$$test_bin"
	done

coverage:
	command -v gcov >/dev/null
	mkdir -p "$(COVERAGE_BUILD_DIR)" "$(COVERAGE_REPORT_DIR)"
	find "$(COVERAGE_BUILD_DIR)" -type f -name '*.gcda' -delete
	$(MAKE) --no-print-directory -B BUILD_DIR="$(COVERAGE_BUILD_DIR)" \
	  CFLAGS="$(CFLAGS) -O0 -g --coverage" LDLIBS="$(LDLIBS) --coverage" test
	gcov -b -o "$(COVERAGE_BUILD_DIR)/obj" $(CORE_SOURCES)
	gcov -b -o "$(COVERAGE_BUILD_DIR)/obj/simulate_entry.gcno" src/simulate.c
	gcov -b -o "$(COVERAGE_BUILD_DIR)/obj/plot_main_entry.gcno" src/plot_main.c
	mv $(COVERAGE_REPORT_FILES) "$(COVERAGE_REPORT_DIR)/"

# Run and plot
run: NAME=default
run: run-one

plot: $(APP_BINS)
	if [[ -z "$(DISTRIBUTION)" ]]; then
	  printf 'Erreur : aucun CSV de distribution fourni. Utilisez make plot DISTRIBUTION=chemin/vers/fichier.csv.\n' >&2
	  exit 1
	fi
	if [[ ! -f "$(DISTRIBUTION)" ]]; then
	  printf 'Erreur : fichier de distribution introuvable : %s\n' "$(DISTRIBUTION)" >&2
	  exit 1
	fi
	png_file="$(PNG)"
	if [[ -z "$$png_file" ]]; then
	  if [[ "$(DISTRIBUTION)" == *.distribution.csv ]]; then
	    png_file="$${DISTRIBUTION%.distribution.csv}.png"
	  else
	    png_file="$${DISTRIBUTION%.csv}.png"
	  fi
	fi
	"$(BUILD_DIR)/plot_main" "$(DISTRIBUTION)" "$(CONFIG)" "$$png_file" "$(PLOT_TYPE)"

run-one: $(APP_BINS)
	test -f "$(CONFIG)"
	out_dir="$(OUTPUT_DIR)/$(NAME)"
	mkdir -p "$$out_dir"
	seed=$$(awk -F= '/^[[:space:]]*seed[[:space:]]*=/{gsub(/[ \t]/, "", $$2); print $$2; exit}' "$(CONFIG)")
	"$(BUILD_DIR)/simulate" "$(CONFIG)" "$$seed" "$$out_dir"
	csv_file=$$(find "$$out_dir" -maxdepth 1 -type f -name '*.csv' ! -name '*.distribution.csv' -print -quit)
	test -n "$$csv_file"
	distribution_file="$${csv_file%.csv}.distribution.csv"
	test -f "$$distribution_file"
	awk -F, 'NR>1 && $$3 > 0 {count++} END {print count+0}' "$$csv_file" > "$$out_dir/stats.txt"
	sorted_csv="$${csv_file}.sorted"
	{ head -n 1 "$$csv_file"; tail -n +2 "$$csv_file" | sort -t, -k1,1nr; } > "$$sorted_csv"
	mv "$$sorted_csv" "$$csv_file"
	png_file="$${csv_file%.csv}.png"
	$(MAKE) --no-print-directory plot DISTRIBUTION="$$distribution_file" CONFIG="$(CONFIG)" PNG="$$png_file"
	printf 'Output: %s\n' "$$out_dir"

run-many: $(APP_BINS)
	set -e
	mkdir -p "$(OUTPUT_DIR)"
	printf '#run seed diff\n' > "$(OUTPUT_DIR)/diffs.txt"
	mkdir -p "$(OUTPUT_DIR)/.run-many"
	for ((seed = 1; seed <= $(N_RUNS); seed++)); do
	  output=$$("$(BUILD_DIR)/simulate" "$(CONFIG_DIR)/config.dat" "$$seed" "$(OUTPUT_DIR)/.run-many")
	  diff=$$(printf '%s\n' "$$output" | awk '/^MC_DIFF /{print $$2; exit}')
	  printf '%d %d %s\n' "$$seed" "$$seed" "$$diff" >> "$(OUTPUT_DIR)/diffs.txt"
	done
	rm -rf "$(OUTPUT_DIR)/.run-many"

# Parameter sweeps and reports
sweep: $(APP_BINS)
	set -e
	case "$(GEOMETRY)" in sphere|cube) ;; *) printf 'Unsupported GEOMETRY: %s\n' "$(GEOMETRY)" >&2; exit 2 ;; esac
	template="$(CONFIG_DIR)/config.template.dat"
	sigma_e=$$(awk -F= '/^[[:space:]]*sigma_e[[:space:]]*=/{gsub(/[ \t]/, "", $$2); print $$2; exit}' "$$template")
	test -n "$$sigma_e"
	for angle in 0 30 60 90; do
	  for tau in 0.01 0.02 0.05 0.1 0.2 0.5 1 2 5 10 20 50 100; do
	    name="angle$${angle}_tau$${tau}"
	    if [[ "$(GEOMETRY)" == cube ]]; then name="cube_$${name}"; fi
	    config_file="$(CONFIG_DIR)/config.$${name}.dat"
	    radius=$$(awk -v geometry="$(GEOMETRY)" -v angle="$$angle" -v tau="$$tau" -v sigma="$$sigma_e" 'BEGIN {if (geometry == "sphere") radius = tau * 3 / (4 * sigma); else {radians = angle * atan2(0, -1) / 180; sine = sin(radians); cosine = cos(radians); if (sine < 0) sine = -sine; if (cosine < 0) cosine = -cosine; radius = tau * (sine + cosine) / (2 * sigma)}; printf "%.8f", radius}')
	    sed -e "s/{{ANGLE}}/$$angle/g" -e "s/{{TAU}}/$$tau/g" -e "s/{{RADIUS}}/$$radius/g" -e "s/{{GEOMETRY}}/$(GEOMETRY)/g" "$$template" > "$$config_file"
	    $(MAKE) --no-print-directory run-one CONFIG="$$config_file" NAME="$$name"
	  done
	done

.replot-sweep: $(APP_BINS)
	set -e
	shopt -s nullglob
	distribution_files=("$(OUTPUT_DIR)"/angle*_tau*/*.distribution.csv "$(OUTPUT_DIR)"/cube_angle*_tau*/*.distribution.csv)
	for distribution_file in "$${distribution_files[@]}"; do
	  experiment_name=$$(basename "$$(dirname "$$distribution_file")")
	  config_file="$(CONFIG_DIR)/config.$${experiment_name}.dat"
	  test -f "$$config_file"
	  png_file="$${distribution_file%.distribution.csv}.png"
	  $(MAKE) --no-print-directory plot DISTRIBUTION="$$distribution_file" CONFIG="$$config_file" PNG="$$png_file" PLOT_TYPE=distribution
	  linear_png_file="$${distribution_file%.distribution.csv}_linear.png"
	  $(MAKE) --no-print-directory plot DISTRIBUTION="$$distribution_file" CONFIG="$$config_file" PNG="$$linear_png_file" PLOT_TYPE=distribution-linear
	  mu_png_file="$${distribution_file%.distribution.csv}_mu.png"
	  $(MAKE) --no-print-directory plot DISTRIBUTION="$$distribution_file" CONFIG="$$config_file" PNG="$$mu_png_file" PLOT_TYPE=distribution-mu
	  mu_linear_png_file="$${distribution_file%.distribution.csv}_mu_linear.png"
	  $(MAKE) --no-print-directory plot DISTRIBUTION="$$distribution_file" CONFIG="$$config_file" PNG="$$mu_linear_png_file" PLOT_TYPE=distribution-mu-linear
	done

report: .replot-sweep
	set -e
	html_escape() {
	  local value=$$1
	  value=$${value//&/\&amp;}
	  value=$${value//</\&lt;}
	  value=$${value//>/\&gt;}
	  value=$${value//\"/\&quot;}
	  value=$${value//\'/\&#39;}
	  printf '%s' "$$value"
	}
	output_dir="$(OUTPUT_DIR)"
	mkdir -p "$$output_dir"
	outfile="$$output_dir/report_interactif.html"
	{
	  printf '%s\n' '<!DOCTYPE html>' '<html lang="fr">' '<head>' '<meta charset="UTF-8">' '<title>Rapport interactif – scatter-cloud-mc</title>' '<link rel="stylesheet" href="../web/report_interactif.css">' '</head>' '<body>' '<h1>Rapport interactif – scatter-cloud-mc</h1>' '<div class="controls"><label>Angle : <select id="angleFilter"><option value="all">Tous</option></select></label>' '<label>Tau : <select id="tauFilter"><option value="all">Tous</option></select></label>' '<label class="scale-toggle" for="scaleFilter"><input id="scaleFilter" type="checkbox" role="switch"><span>Échelle logarithmique</span></label>' '<label class="scale-toggle" for="cosAlphaDFilter"><input id="cosAlphaDFilter" type="checkbox" role="switch"><span>Abscisse cos(α_d)</span></label></div>' '<div class="grid" id="grid">'
	  while IFS= read -r image; do
	    directory=$$(dirname "$$image")
	    directory_name=$$(basename "$$directory")
	    geometry=sphere
	    if [[ "$$directory_name" == cube_* ]]; then
	      geometry=cube
	      directory_name=$${directory_name#cube_}
	    fi
	    angle=$${directory_name#angle}
	    angle=$${angle%%_tau*}
	    tau=$${directory_name#*_tau}
	    stats_file="$$directory/stats.txt"
	    photons_detected=$$(cat "$$stats_file" 2>/dev/null || printf '?')
	    image_relative=$${image#"$$output_dir"/}
	    linear_image="$${image%.png}_linear.png"
	    if [[ ! -f "$$linear_image" ]]; then linear_image="$$image"; fi
	    linear_image_relative=$${linear_image#"$$output_dir"/}
	    mu_image="$${image%.png}_mu.png"
	    if [[ ! -f "$$mu_image" ]]; then mu_image="$$image"; fi
	    mu_image_relative=$${mu_image#"$$output_dir"/}
	    mu_linear_image="$${image%.png}_mu_linear.png"
	    if [[ ! -f "$$mu_linear_image" ]]; then mu_linear_image="$$linear_image"; fi
	    mu_linear_image_relative=$${mu_linear_image#"$$output_dir"/}
	    graph_type="Distribution"
	    if [[ "$$image" == *_phase.png ]]; then graph_type="Fonction de phase"; fi
	    caption="$${graph_type}, géométrie = $${geometry}, angle = $${angle}°, tau = $${tau}, N détectés = $${photons_detected}"
	    escaped_angle=$$(html_escape "$$angle")
	    escaped_tau=$$(html_escape "$$tau")
	    escaped_image=$$(html_escape "$$image_relative")
	    escaped_linear_image=$$(html_escape "$$linear_image_relative")
	    escaped_mu_image=$$(html_escape "$$mu_image_relative")
	    escaped_mu_linear_image=$$(html_escape "$$mu_linear_image_relative")
	    escaped_caption=$$(html_escape "$$caption")
	    printf '  <div class="card" data-angle="%s" data-tau="%s" data-alpha-d-log-src="%s" data-alpha-d-linear-src="%s" data-mu-log-src="%s" data-mu-linear-src="%s"><img src="%s" alt="%s"><div class="caption">%s<br><code>%s</code></div></div>\n' "$$escaped_angle" "$$escaped_tau" "$$escaped_image" "$$escaped_linear_image" "$$escaped_mu_image" "$$escaped_mu_linear_image" "$$escaped_linear_image" "$$escaped_caption" "$$escaped_caption" "$$escaped_linear_image"
	  done < <(find "$$output_dir" -type f -name '*.png' ! -name '*_phase.png' ! -name '*_linear.png' ! -name '*_mu.png' | sort)
	  printf '%s\n' '</div>' '<div id="lightbox"><img id="lightbox-img" src=""></div>' '<script src="../web/report_interactif.js"></script>' '</body>' '</html>'
	} > "$$outfile"
	printf 'Report: %s\n' "$$outfile"

full: build test sweep report

# Formatting and quality checks
format:
	$(CLANG_FORMAT) -i $(FORMAT_SOURCES)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_SOURCES)

quality: format-check
	cppcheck --enable=warning,performance,portability --error-exitcode=1 -Iinclude $(wildcard src/*.c)
	clang-tidy --quiet $(wildcard src/*.c) -- -std=c11 -D_DEFAULT_SOURCE -Iinclude

# Cleanup
clean:
	rm -rf "$(BUILD_DIR)" "$(OUTPUT_DIR)" "$(COVERAGE_REPORT_DIR)"
	find "$(CONFIG_DIR)" -maxdepth 1 -type f -name 'config.*.dat' ! -name 'config.dat' ! -name 'config.template.dat' -delete
