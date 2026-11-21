# Git Workflow - scatter-cloud-mc

## Règles de Commit

- **Commits atomiques** : une seule modification logique par commit
- **Messages impératifs courts** : ex: "fix: corriger warning clang-tidy", "docs: ajouter diagramme architecture"
- **Validation obligatoire** : `make test` + `make quality` doivent passer avant commit
- **Pas de commit sans validation utilisateur explicite**

## Branches

- `main` : branche stable, protégée (CI requis)
- Features : branches courtes, nommage `feature/<sujet>` ou `fix/<sujet>`
- Pas de commit direct sur `main`

## Pull Requests

- Description claire du changement
- Tests ajoutés/mis à jour si comportement modifié
- `make quality` vert dans CI
- Revue avant merge (même en solo - auto-revue)