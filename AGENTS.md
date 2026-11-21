# AGENTS.md - Directives Générales pour les Agents

## Philosophie de Développement

**Qualité avant quantité.** Code correct, lisible et maintenable plutôt que livraison rapide.

**Tests comme documentation.** Chaque test décrit un comportement observable. Les tests échouent pour de bonnes raisons.

**Itérations courtes.** Une tâche = un commit cohérent. Valider souvent (tests + quality).

## Standards de Code

- **C11** standard, compilateur GCC/Clang
- **clang-format** : style projet (voir `.clang-format`)
- **cppcheck** + **clang-tidy** : zéro warning toléré (`make quality`)
- **_Static_assert** pour tailles de structures (ABI stability)
- **Const-correctness** : `const` par défaut, retirer si nécessaire
- **Cartouche d'en-tête** : tout fichier `.c`/`.h` commence par un bloc commentaire :
  ```c
  /*
   * <Nom du fichier> - <Description courte (1 ligne)>
   * <Description détaillée si nécessaire>
   *
   * Partie de <Nom du projet>
   */
  ```
  Exemple :
  ```c
  /*
   * simulation.c - Boucle principale de transport de photons Monte Carlo
   * Gère le cycle de vie des photons, diffusion, absorption, roulette russe,
   * et accumule les statistiques énergétiques avec sommation Kahan.
   *
   * Partie de scatter-cloud-mc
   */
  ```

## Tests (Structure AAA)

```text
Given : entrées, résultats attendus, contexte (const quand possible)
When  : action principale unique
Then  : vérifications observables + diagnostic utile
```

- Un comportement = un test
- Cas nominaux, limites, erreurs séparés
- État frais par test (pas de dépendance d'ordre)
- Helpers `setup_*`, `assert_*` dans le fichier de test
- Runner partagé : `PASS`/`FAIL`/`SKIP`, continue après échec

## Workflow Git

- Commits atomiques, messages impératifs courts
- `make test` + `make quality` avant commit
- Pas de commit sans validation utilisateur explicite

## Documentation (Markdown)

- **markdownlint** : zéro erreur tolérée (CI)
- Blocs de code clôturés : **toujours spécifier un langage** (`bash`, `c`, `text`, etc.)
- Fin de fichier : **exactement une newline** (pas 0, pas >1)
- Titres : une seule ligne `#` par niveau, pas de saut de ligne manquant
- Tables : **espaces autour des pipes** sur toutes les lignes (`| Col1 | Col2 |` pas `|Col1|Col2|`), y compris la ligne de séparation (`| --- | --- |`)

## Outils

- **Recherche** : `grep`/`glob` (pas `find`/`ls`)
- **Édition** : `edit` (pas `sed`/`awk`)
- **Lecture** : `read` (pas `cat`)
- **Processus longs** : `background_process` (servers, watchers)
- **Web** : `webfetch`/`websearch` pour info externes



