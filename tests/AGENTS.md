# Consignes pour les tests

**Structure AAA.** Chaque test suit AAA, avec des sections `Given`, `When` et
`Then`.

**Scénarios.** Garder chaque test centré sur un comportement observable.
Séparer le cas nominal, les cas limites et les erreurs.

**Assertions.** Plusieurs assertions sont acceptables si elles vérifient
ensemble le même comportement. Éviter de regrouper des vérifications sans lien.

**Noms et descriptions.** Nommer chaque test selon le comportement attendu.
Précéder chaque fonction de test d'une brève description du comportement
vérifié. Ne pas ajouter ces descriptions aux helpers.

**Given.** Déclarer les entrées, les résultats attendus et le contexte du
scénario. Rendre les résultats attendus `const` lorsque c'est approprié, puis
les réutiliser dans `Then`.

**État des tests.** Créer un contexte mutable neuf pour chaque test. Les
fixtures immuables peuvent être partagées. Ne pas dépendre de l'ordre
d'exécution ni de l'état laissé par un autre test.

**Préparation.** Garder les préparations propres à un scénario dans son
`Given`. Factoriser dans un helper `setup_*` seulement si cela réduit une
duplication ou clarifie le test. Appeler ce helper depuis le `Given` de chaque
test concerné. Il doit créer un état propre sans exécuter le comportement testé.

**Organisation.** Séparer helpers, cas de test et runner (`main`). Placer les
helpers avant les cas. Réserver le préfixe `test_` aux fonctions de test. Le
runner n'enregistre que les cas, jamais les helpers.

**Langue.** Rédiger les commentaires des tests en français. Les balises AAA
`Given`, `When` et `Then` peuvent rester en anglais.

**Helpers.** Choisir un nom qui décrit leur rôle (`setup_*`, `assert_*`,
`capture_*`, etc.). Les garder dans le fichier de test tant qu'ils ne servent
qu'à celui-ci; les partager seulement s'ils servent plusieurs suites.

**When.** Effectuer l'action principale, en limitant la préparation et les
actions sans rapport.

**Then.** Vérifier les résultats observables et fournir un diagnostic utile
en cas d'échec.

**Runner.** Déclarer les cas dans un tableau et déléguer l'exécution, le
reporting et le bilan au runner partagé. Utiliser les statuts `PASS`, `FAIL` et
`SKIP`. Le runner identifie chaque cas, poursuit après un échec et calcule les
totaux sans compteur manuel dans la suite.
