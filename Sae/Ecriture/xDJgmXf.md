# Read Files

## Modifiez `TXT_FILE_PATH` et `GIF_FILE_PATH` dans `main.h` selon l'emplacement de vos fichiers `.txt` et `.gif` sur votre machine. Ainsi que `INSTDIR` dans `Makefile`.

Ce programme a ete realiser pour tester la modification du fichier texte en fonction de la taille d'un fichier `.gif` (ou autre fichier binaire).
Le programme lit une liste d'adresses dans `test.txt`, calcule une nouvelle adresse alignee (512 octets), puis ecrit la liste mise a jour.

## Format attendu pour le fichier texte
- Premiere ligne: nombre d'entrees (ex: `4`)
- Puis une entree par ligne: 4 octets en hexadecimal colles (ex: `00112233`)

## Utilisation
1. Verifier les chemins dans `main.h` et dans le `Makefile`
2. `make install`
3. Lancer le binaire genéré.
4. Recommande: `cat test.txt` avant et apres l'execution pour verifier les modifications.

## Exemple rapide
Contenu de `test.txt`:
```
2
00112233
44556677
```

Le programme ajoute une nouvelle entrée calculée à la fin du fichier.
