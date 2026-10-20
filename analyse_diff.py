import argparse
import statistics


def read_photon_count(config_path):
    with open(config_path, encoding="utf-8") as config_file:
        for line in config_file:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, value = line.split("=", 1)
            if key.strip() == "n_photons":
                photon_count = int(value.strip())
                if photon_count <= 0:
                    raise ValueError("n_photons doit être strictement positif")
                return photon_count
    raise ValueError(f"n_photons absent de {config_path}")


def read_differences(input_path):
    differences = []
    with open(input_path, encoding="utf-8") as input_file:
        for line_number, line in enumerate(input_file, start=1):
            fields = line.split()
            if not fields or fields[0].startswith("#"):
                continue
            if len(fields) != 3:
                raise ValueError(f"format invalide à la ligne {line_number} de {input_path}")
            differences.append(float(fields[2]))
    if not differences:
        raise ValueError(f"aucun écart trouvé dans {input_path}")
    return differences


def main():
    parser = argparse.ArgumentParser(description="Analyse les résidus de make run-many.")
    parser.add_argument("--input", default="output/diffs.txt", help="fichier des résidus")
    parser.add_argument("--config", default="config/config.dat", help="configuration utilisée")
    args = parser.parse_args()

    try:
        photon_count = read_photon_count(args.config)
        differences = read_differences(args.input)
    except (OSError, ValueError) as error:
        parser.error(str(error))

    mean = statistics.fmean(differences)
    standard_deviation = statistics.stdev(differences) if len(differences) > 1 else None
    print("Nombre d'échantillons :", len(differences))
    print("Photons injectés par simulation :", photon_count)
    print("Résidu moyen :", mean)
    if standard_deviation is None:
        print("Écart-type : non défini (un seul échantillon)")
    else:
        print("Écart-type échantillonnal :", standard_deviation)
        print("Erreur-type de la moyenne :", standard_deviation / len(differences) ** 0.5)
    print("Résidu relatif moyen :", mean / photon_count)
    if standard_deviation is not None:
        print("Écart-type relatif :", standard_deviation / photon_count)


if __name__ == "__main__":
    main()
