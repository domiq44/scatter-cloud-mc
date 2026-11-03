SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.SILENT:
.DEFAULT_GOAL := help

# Toolchain
CC := gcc
CLANG_FORMAT ?= clang-format

# Directories
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
COVERAGE_BUILD_DIR := $(BUILD_DIR)/coverage
COVERAGE_REPORT_DIR := coverage
CONFIG_DIR := config
OUTPUT_DIR := output
QUICK_OUTPUT_DIR ?= output/quick
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
ANGLES ?= 0 30 60 70 80 85 90
TAUS ?= 0.01 0.02 0.05 0.1 0.2 0.5 1 2 5 10 20 50 100
N_PHOTONS ?= $(shell awk -F= '/^[[:space:]]*n_photons[[:space:]]*=/{gsub(/[ \t]/, "", $$2); print $$2; exit}' "$(CONFIG_DIR)/config.template.dat")
RESUME ?= 0
ESTIMATE_SECONDS_PER_PHOTON ?=
QUICK_N_PHOTONS ?= 2000
QUICK_ANGLES ?= 0 60 90
QUICK_TAUS ?= 0.1 1 10
QUICK_WEB_ASSET_PREFIX ?= ../../web
WEB_ASSET_PREFIX ?= ../web
DISTRIBUTION ?=
PNG ?=
PLOT_TYPE ?= both

.PHONY: all help \
	build test coverage format format-check \
	run run-one plot run-many \
	sweep sweep-plan quick report full quality clean clean-output clean-coverage clean-sweep-configs clean-all .replot-sweep

all: build test

help:
	@printf '%s\n' \
	  'Build and tests:' \
	  '  make build           Build application and tests' \
	  '  make test            Build and run every test suite' \
	  '  make coverage        Run tests with gcov and write coverage/*.gcov' \
	  '  make format          Apply clang-format before pushing' \
	  '  make format-check    Verify formatting without modifying files' \
	  'Run and plot:' \
	  '  make run             Run the default simulation and plot' \
	  '  make run-one CONFIG=... NAME=... Run one named experiment' \
	  '  make plot DISTRIBUTION=... [CONFIG=...] [PNG=...] Plot existing distribution CSV' \
	  '    PLOT_TYPE=distribution|distribution-linear|distribution-mu|distribution-mu-linear|phase|both (default both)' \
	  '  make run-many N_RUNS=20 Check seed stability' \
	  'Sweeps and reports:' \
	  '  make sweep-plan         Preview sweep size and approximate duration' \
	  '  make sweep [GEOMETRY=sphere|cube|both] [N_PHOTONS=...] [ANGLES="..."] [TAUS="..."]' \
	  '  make quick              Run a small sweep in output/quick/ and generate its report' \
	  '    RESUME=1 skips cases with matching saved configurations' \
	  '    ESTIMATE_SECONDS_PER_PHOTON=... overrides the local timing estimate' \
	  '  make report          Regenerate sweep plots and the HTML report' \
	  '  make full            Build and test, then sweep and report in order (safe with -j)' \
	  'Quality and cleanup:' \
	  '  make quality         Check formatting and run static analysis' \
	  '  make clean           Remove build files only' \
	  '  make clean-output    Remove output/ and the quick-sweep output' \
	  '  make clean-coverage  Remove coverage reports' \
	  '  make clean-sweep-configs  Remove generated sweep configs from config/' \
	  '  make clean-all       Remove build files, generated configs, outputs and coverage'

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
	simulation_output=$$("$(BUILD_DIR)/simulate" "$(CONFIG)" "$$seed" "$$out_dir")
	printf '%s\n' "$$simulation_output"
	csv_file=$$(printf '%s\n' "$$simulation_output" | sed -n 's/^Fichier généré : //p')
	test -n "$$csv_file"
	distribution_file="$${csv_file%.csv}.distribution.csv"
	test -f "$$distribution_file"
	awk -F, 'NR>1 {count++} END {print count+0}' "$$csv_file" > "$$out_dir/stats.txt"
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
sweep-plan:
	set -e
	case "$(GEOMETRY)" in
	  sphere) geometries=(sphere) ;;
	  cube) geometries=(cube) ;;
	  both) geometries=(sphere cube) ;;
	  *) printf 'Unsupported GEOMETRY: %s (expected sphere, cube, or both)\n' "$(GEOMETRY)" >&2; exit 2 ;;
	esac
	read -r -a angles <<< "$(ANGLES)"
	read -r -a taus <<< "$(TAUS)"
	photon_count="$(N_PHOTONS)"
	if ! [[ "$$photon_count" =~ ^[1-9][0-9]*$$ ]]; then
	  printf 'N_PHOTONS must be a positive integer: %s\n' "$$photon_count" >&2
	  exit 2
	fi
	if [[ "$(RESUME)" != 0 && "$(RESUME)" != 1 ]]; then
	  printf 'RESUME must be 0 or 1: %s\n' "$(RESUME)" >&2
	  exit 2
	fi
	if [[ $${#angles[@]} -eq 0 || $${#taus[@]} -eq 0 ]]; then
	  printf 'ANGLES and TAUS must each contain at least one value\n' >&2
	  exit 2
	fi
	for angle in "$${angles[@]}"; do
	  if ! [[ "$$angle" =~ ^[0-9]+([.][0-9]+)?$$ ]] || ! awk -v value="$$angle" 'BEGIN {exit !(value <= 90)}'; then
	    printf 'Invalid angle (expected a number from 0 to 90): %s\n' "$$angle" >&2
	    exit 2
	  fi
	done
	for tau in "$${taus[@]}"; do
	  if ! [[ "$$tau" =~ ^[0-9]+([.][0-9]+)?$$ ]] || ! awk -v value="$$tau" 'BEGIN {exit !(value > 0)}'; then
	    printf 'Invalid optical thickness (expected a positive number): %s\n' "$$tau" >&2
	    exit 2
	  fi
	done
	case_count=$$(($${#geometries[@]} * $${#angles[@]} * $${#taus[@]}))
	printf 'Géométries : %s\n' "$(GEOMETRY)"
	printf 'Photons par cas : %s\n' "$$photon_count"
	printf 'Angles : %s\n' "$(ANGLES)"
	printf 'Tau : %s\n' "$(TAUS)"
	printf 'Nombre de simulations : %d\n' "$$case_count"
	estimate_rate="$(ESTIMATE_SECONDS_PER_PHOTON)"
	if [[ -z "$$estimate_rate" ]]; then
	  timing_files=("$(OUTPUT_DIR)/.sweep-timings.tsv" "$(QUICK_OUTPUT_DIR)/.sweep-timings.tsv")
	  available_timing_files=()
	  for timing_file in "$${timing_files[@]}"; do
	    if [[ -f "$$timing_file" ]]; then available_timing_files+=("$$timing_file"); fi
	  done
	  if [[ $${#available_timing_files[@]} -gt 0 ]]; then
	    estimate_rate=$$(awk -F '\t' '$$4 ~ /^[0-9]+$$/ && $$4 > 0 && $$5 ~ /^[0-9]+([.][0-9]+)?$$/ {seconds += $$5; photons += $$4} END {if (photons > 0) printf "%.9g", seconds / photons}' "$${available_timing_files[@]}")
	  fi
	fi
	if [[ -n "$$estimate_rate" ]]; then
	  if ! [[ "$$estimate_rate" =~ ^[0-9]+([.][0-9]+)?$$ ]] || ! awk -v value="$$estimate_rate" 'BEGIN {exit !(value > 0)}'; then
	    printf 'ESTIMATE_SECONDS_PER_PHOTON must be a positive number: %s\n' "$$estimate_rate" >&2
	    exit 2
	  fi
	  estimate_seconds=$$(awk -v rate="$$estimate_rate" -v cases="$$case_count" -v photons="$$photon_count" 'BEGIN {printf "%.0f", rate * cases * photons}')
	  estimate_minutes=$$(( (estimate_seconds + 59) / 60 ))
	  estimate_rate_display=$$(awk -v rate="$$estimate_rate" 'BEGIN {printf "%.4g", rate}')
	  if [[ "$$estimate_minutes" -lt 60 ]]; then
	    printf 'Durée estimée : environ %d min (basée sur %s s/phot.)\n' "$$estimate_minutes" "$$estimate_rate_display"
	  else
	    printf 'Durée estimée : environ %d h %02d min (basée sur %s s/phot.)\n' \
	      "$$((estimate_minutes / 60))" "$$((estimate_minutes % 60))" "$$estimate_rate_display"
	  fi
	else
	  printf 'Durée estimée : indisponible (lancez make quick pour calibrer ou fournissez ESTIMATE_SECONDS_PER_PHOTON)\n'
	fi

sweep: $(APP_BINS)
	$(MAKE) --no-print-directory sweep-plan GEOMETRY="$(GEOMETRY)" N_PHOTONS="$(N_PHOTONS)" ANGLES="$(ANGLES)" TAUS="$(TAUS)" RESUME="$(RESUME)" >/dev/null
	set -e
	case "$(GEOMETRY)" in
	  sphere|cube) geometries=("$(GEOMETRY)") ;;
	  both) geometries=(sphere cube) ;;
	  *) printf 'Unsupported GEOMETRY: %s (expected sphere, cube, or both)\n' "$(GEOMETRY)" >&2; exit 2 ;;
	esac
	read -r -a angles <<< "$(ANGLES)"
	read -r -a taus <<< "$(TAUS)"
	photon_count="$(N_PHOTONS)"
	template="$(CONFIG_DIR)/config.template.dat"
	sigma_e=$$(awk -F= '/^[[:space:]]*sigma_e[[:space:]]*=/{gsub(/[ \t]/, "", $$2); print $$2; exit}' "$$template")
	test -n "$$sigma_e"
	for geometry in "$${geometries[@]}"; do
	  for angle in "$${angles[@]}"; do
	    for tau in "$${taus[@]}"; do
	      base_name="angle$${angle}_tau$${tau}"
	      if [[ "$$geometry" == cube ]]; then base_name="cube_$${base_name}"; fi
	      temp_config=$$(mktemp "$(BUILD_DIR)/sweep-config.XXXXXX")
	      radius=$$(awk -v geometry="$$geometry" -v angle="$$angle" -v tau="$$tau" -v sigma="$$sigma_e" 'BEGIN {if (geometry == "sphere") radius = tau * 3 / (4 * sigma); else {radians = angle * atan2(0, -1) / 180; sine = sin(radians); cosine = cos(radians); if (sine < 0) sine = -sine; if (cosine < 0) cosine = -cosine; radius = tau * (sine + cosine) / (2 * sigma)}; printf "%.8f", radius}')
	      sed -e "s/^[[:space:]]*n_photons[[:space:]]*=.*/n_photons = $$photon_count/" \
	        -e "s/{{ANGLE}}/$$angle/g" -e "s/{{TAU}}/$$tau/g" -e "s/{{RADIUS}}/$$radius/g" \
	        -e "s/{{GEOMETRY}}/$$geometry/g" "$$template" > "$$temp_config"
	      config_signature=$$(cksum "$$temp_config" | awk '{printf "%s_%s", $$1, $$2}')
	      name="$${base_name}_cfg$${config_signature}"
	      config_file="$(CONFIG_DIR)/config.$${name}.dat"
	      mv "$$temp_config" "$$config_file"
	      experiment_dir="$(OUTPUT_DIR)/$$name"
	      snapshot="$$experiment_dir/sweep-config.dat"
	      distribution_file=$$(find "$$experiment_dir" -maxdepth 1 -type f -name '*.distribution.csv' -print -quit 2>/dev/null || true)
	      if [[ "$(RESUME)" == 1 && -n "$$distribution_file" && -f "$$snapshot" ]] && cmp -s "$$config_file" "$$snapshot"; then
	        printf 'Skip %s (configuration déjà calculée)\n' "$$name"
	      else
	        start_ns=$$(date +%s%N)
	        $(MAKE) --no-print-directory run-one CONFIG="$$config_file" NAME="$$name" OUTPUT_DIR="$(OUTPUT_DIR)" GEOMETRY="$$geometry"
	        end_ns=$$(date +%s%N)
	        elapsed_seconds=$$(awk -v start="$$start_ns" -v end="$$end_ns" 'BEGIN {printf "%.6f", (end - start) / 1000000000}')
	        printf '%s\t%s\t%s\t%s\t%s\n' "$$geometry" "$$angle" "$$tau" "$$photon_count" "$$elapsed_seconds" >> "$(OUTPUT_DIR)/.sweep-timings.tsv"
	        cp "$$config_file" "$$snapshot"
	      fi
	    done
	  done
	done

quick:
	$(MAKE) --no-print-directory sweep GEOMETRY="$(GEOMETRY)" OUTPUT_DIR="$(QUICK_OUTPUT_DIR)" N_PHOTONS="$(QUICK_N_PHOTONS)" ANGLES="$(QUICK_ANGLES)" TAUS="$(QUICK_TAUS)" RESUME="$(RESUME)"
	$(MAKE) --no-print-directory report OUTPUT_DIR="$(QUICK_OUTPUT_DIR)" WEB_ASSET_PREFIX="$(QUICK_WEB_ASSET_PREFIX)"

.replot-sweep: $(APP_BINS)
	set -e
	shopt -s nullglob
	distribution_files=("$(OUTPUT_DIR)"/angle*_tau*/*.distribution.csv "$(OUTPUT_DIR)"/cube_angle*_tau*/*.distribution.csv)
	for distribution_file in "$${distribution_files[@]}"; do
	  experiment_name=$$(basename "$$(dirname "$$distribution_file")")
	  config_file="$(OUTPUT_DIR)/$$experiment_name/sweep-config.dat"
	  if [[ ! -f "$$config_file" ]]; then config_file="$(CONFIG_DIR)/config.$${experiment_name}.dat"; fi
	  if [[ ! -f "$$config_file" ]]; then
	    printf 'Skipping %s: saved sweep configuration not found\n' "$$experiment_name" >&2
	    continue
	  fi
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
	shopt -s nullglob
	flux_files=("$$output_dir"/angle*_tau*/*.flux.csv "$$output_dir"/cube_angle*_tau*/*.flux.csv)
	photon_count_for_file() {
	  local input_file=$$1
	  local case_dir case_name config_file photon_count
	  case_dir=$$(dirname "$$input_file")
	  case_name=$$(basename "$$case_dir")
	  config_file="$$case_dir/sweep-config.dat"
	  if [[ ! -f "$$config_file" ]]; then config_file="$(CONFIG_DIR)/config.$${case_name}.dat"; fi
	  if [[ -f "$$config_file" ]]; then
	    photon_count=$$(awk -F= '/^[[:space:]]*n_photons[[:space:]]*=/{gsub(/[ \t]/, "", $$2); print $$2; exit}' "$$config_file")
	  fi
	  if [[ -z "$$photon_count" ]]; then
	    photon_count=$$(basename "$$input_file" | sed -n 's/.*_N\([0-9][0-9]*\)[_.].*/\1/p')
	  fi
	  printf '%s' "$${photon_count:-unknown}"
	}
	declare -A seen_photon_counts=()
	photon_counts=()
	for flux_file in "$${flux_files[@]}"; do
	  photon_count=$$(photon_count_for_file "$$flux_file")
	  if [[ -z "$${seen_photon_counts[$$photon_count]+x}" ]]; then
	    seen_photon_counts[$$photon_count]=1
	    photon_counts+=("$$photon_count")
	  fi
	done
	reflectance_images=()
	rm -f "$$output_dir"/reflectance_sphere_N*.dat "$$output_dir"/reflectance_cube_N*.dat \
	  "$$output_dir"/reflectance_sphere_N*.png "$$output_dir"/reflectance_cube_N*.png
	for geometry in sphere cube; do
	  for photon_count in "$${photon_counts[@]}"; do
	    data_file="$$output_dir/reflectance_$${geometry}_N$${photon_count}.dat"
	    : > "$$data_file"
	    for flux_file in "$${flux_files[@]}"; do
	      experiment_dir=$$(basename "$$(dirname "$$flux_file")")
	      file_geometry=sphere
	      if [[ "$$experiment_dir" == cube_* ]]; then
	        file_geometry=cube
	        experiment_dir=$${experiment_dir#cube_}
	      fi
	      if [[ "$$file_geometry" != "$$geometry" ]] || [[ "$$(photon_count_for_file "$$flux_file")" != "$$photon_count" ]]; then continue; fi
	      angle=$${experiment_dir#angle}
	      angle=$${angle%%_tau*}
	      tau=$${experiment_dir#*_tau}
	      tau=$${tau%%_*}
	      reflectance=$$(awk -F, '$$1 == "diffuse_upward" {print $$3; exit}' "$$flux_file")
	      if [[ -n "$$reflectance" ]]; then
	        printf '%s %s %s\n' "$$angle" "$$tau" "$$reflectance" >> "$$data_file"
	      fi
	    done
	    if [[ -s "$$data_file" ]]; then
	      sort -g -k1,1 -k2,2 "$$data_file" -o "$$data_file"
	      geometry_label=Sphère
	      if [[ "$$geometry" == cube ]]; then geometry_label=Cube; fi
	      output_file="$$output_dir/reflectance_$${geometry}_N$${photon_count}.png"
	      gnuplot -e "data_file='$$data_file'; output_file='$$output_file'; geometry='$$geometry_label, N=$$photon_count'" \
	        gnuplot/directional_reflectance.gp.in
	      reflectance_images+=("$$output_file")
	    fi
	  done
	done
	outfile="$$output_dir/report_interactif.html"
	{
	  printf '%s\n' '<!DOCTYPE html>' '<html lang="fr">' '<head>' '<meta charset="UTF-8">' '<title>Rapport interactif – scatter-cloud-mc</title>' '<link rel="stylesheet" href="$(WEB_ASSET_PREFIX)/report_interactif.css">' '</head>' '<body>' '<h1>Rapport interactif – scatter-cloud-mc</h1>'
	  printf '%s\n' '<div class="controls"><label>Forme : <select id="geometryFilter"><option value="all">Toutes</option><option value="sphere">Sphère</option><option value="cube">Cube</option></select></label>' '<label>Angle : <select id="angleFilter"><option value="all">Tous</option></select></label>' '<label>Tau : <select id="tauFilter"><option value="all">Tous</option></select></label>' '<label>Photons injectés : <select id="photonFilter"><option value="all">Tous</option></select></label>' '<label class="scale-toggle" for="scaleFilter"><input id="scaleFilter" type="checkbox" role="switch"><span>Échelle logarithmique</span></label>' '<label class="scale-toggle" for="cosAlphaDFilter"><input id="cosAlphaDFilter" type="checkbox" role="switch"><span>Abscisse cos(α_d)</span></label></div>'
	  if [[ $${#reflectance_images[@]} -gt 0 ]]; then
	    printf '%s\n' '<section class="reflectance"><h2>Réflectance directionnelle</h2>' '<p>R<sub>dir</sub> = flux diffus montant / flux incident dans la direction θ<sub>0</sub>.</p>' '<div class="reflectance-grid">'
	    for reflectance_image in "$${reflectance_images[@]}"; do
	      png_file=$$(basename "$$reflectance_image")
	      image_stem=$${png_file%.png}
	      geometry=$${image_stem#reflectance_}
	      geometry=$${geometry%%_N*}
	      photon_count=$${image_stem##*_N}
	      label=Sphère
	      if [[ "$$geometry" == cube ]]; then label=Cube; fi
	      printf '  <figure data-geometry="%s" data-photons="%s"><figcaption>%s, N = %s</figcaption><img src="%s" alt="Réflectance directionnelle %s, N = %s"></figure>\n' "$$geometry" "$$photon_count" "$$label" "$$photon_count" "$$png_file" "$$label" "$$photon_count"
	    done
	    printf '%s\n' '</div></section>'
	  fi
	  printf '%s\n' '<div class="grid" id="grid">'
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
	    tau=$${tau%%_*}
	    stats_file="$$directory/stats.txt"
	    photons_detected=$$(cat "$$stats_file" 2>/dev/null || printf '?')
	    config_file="$$directory/sweep-config.dat"
	    if [[ ! -f "$$config_file" ]]; then config_file="$(CONFIG_DIR)/config.$${directory_name}.dat"; fi
	    photons_injected=$$(awk -F= '/^[[:space:]]*n_photons[[:space:]]*=/{gsub(/[ \t]/, "", $$2); print $$2; exit}' "$$config_file" 2>/dev/null || true)
	    if [[ -z "$$photons_injected" ]]; then
	      photons_injected=$$(basename "$$image" | sed -n 's/.*_N\([0-9][0-9]*\)_bins.*/\1/p')
	    fi
	    if [[ -z "$$photons_injected" ]]; then photons_injected=?; fi
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
	    caption="$${graph_type}, géométrie = $${geometry}, angle = $${angle}°, tau = $${tau}, détectés = $${photons_detected} / injectés = $${photons_injected}"
	    escaped_angle=$$(html_escape "$$angle")
	    escaped_tau=$$(html_escape "$$tau")
	    escaped_image=$$(html_escape "$$image_relative")
	    escaped_linear_image=$$(html_escape "$$linear_image_relative")
	    escaped_mu_image=$$(html_escape "$$mu_image_relative")
	    escaped_mu_linear_image=$$(html_escape "$$mu_linear_image_relative")
	    escaped_caption=$$(html_escape "$$caption")
	    escaped_photons=$$(html_escape "$$photons_injected")
	    printf '  <div class="card" data-geometry="%s" data-angle="%s" data-tau="%s" data-photons="%s" data-alpha-d-log-src="%s" data-alpha-d-linear-src="%s" data-mu-log-src="%s" data-mu-linear-src="%s"><img src="%s" alt="%s"><div class="caption">%s</div></div>\n' "$$geometry" "$$escaped_angle" "$$escaped_tau" "$$escaped_photons" "$$escaped_image" "$$escaped_linear_image" "$$escaped_mu_image" "$$escaped_mu_linear_image" "$$escaped_linear_image" "$$escaped_caption" "$$escaped_caption"
	  done < <(find "$$output_dir" -maxdepth 2 -type f -name '*.png' \
	    \( -path "$$output_dir/angle*_tau*/*" -o -path "$$output_dir/cube_angle*_tau*/*" \) \
	    ! -name '*_phase.png' ! -name '*_linear.png' ! -name '*_mu.png' \
	    ! -name 'reflectance_*.png' | sort)
	  printf '%s\n' '</div>' '<div id="lightbox"><img id="lightbox-img" src=""></div>' '<script src="$(WEB_ASSET_PREFIX)/report_interactif.js"></script>' '</body>' '</html>'
	} > "$$outfile"
	printf 'Report: %s\n' "$$outfile"

full: build test
	$(MAKE) --no-print-directory sweep GEOMETRY="$(GEOMETRY)" OUTPUT_DIR="$(OUTPUT_DIR)" CONFIG_DIR="$(CONFIG_DIR)" N_PHOTONS="$(N_PHOTONS)" ANGLES="$(ANGLES)" TAUS="$(TAUS)" RESUME="$(RESUME)"
	$(MAKE) --no-print-directory report OUTPUT_DIR="$(OUTPUT_DIR)" CONFIG_DIR="$(CONFIG_DIR)" WEB_ASSET_PREFIX="$(WEB_ASSET_PREFIX)"

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
	rm -rf "$(BUILD_DIR)"

clean-sweep-configs:
	find "$(CONFIG_DIR)" -maxdepth 1 -type f \
	  \( -name 'config.angle*_tau*.dat' -o -name 'config.cube_angle*_tau*.dat' \) -delete

clean-output:
	rm -rf "$(OUTPUT_DIR)" "$(QUICK_OUTPUT_DIR)"

clean-coverage:
	rm -rf "$(COVERAGE_REPORT_DIR)" "$(COVERAGE_BUILD_DIR)"

clean-all: clean clean-sweep-configs clean-coverage clean-output
	rm -rf "$(OUTPUT_DIR)" "$(QUICK_OUTPUT_DIR)" "$(COVERAGE_REPORT_DIR)"
