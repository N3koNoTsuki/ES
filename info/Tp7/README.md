# TP7 - OLED + GPIO

## Objectif
Piloter l'ecran OLED via RS-232 et changer de GIF avec 4 boutons GPIO.

## Fichiers
- `info/Tp7/oled.c`: programme principal (boucle d'affichage, gestion signaux, fork).
- `info/Tp7/oled_fct.c`: fonctions bas niveau (RS-232, GPIO, commandes OLED).
- `info/Tp7/oled.h`: types, constantes et prototypes.
- `info/Tp7/Makefile`: build et install.

## Build
```sh
make
```

## Execution
```sh
./oled
```

## Peripheriques attendus
- OLED: `/dev/ttyAL0`
- Boutons: `/dev/gpiochip2` (2 lignes), `/dev/gpiochip3` (2 lignes)

## Installation (cible)
```sh
make install prefix=/chemin/vers/rootfs
```
