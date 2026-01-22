# README — WriteGif (upload + lecture d’animation GIF sur écran OLED via RS-232)

- je recommande d'utiliser l'extension `Simply Markdown Viewer` sous Visual pour un meilleur affichage ou tout viewer de .md

## Résumé

`./WriteGif` est un programme Linux (cible ARM) qui :

- Convertit un GIF en flux vidéo brut RGB565BE + en-tête.
- Upload ce flux dans la mémoire "media" de l’écran OLED via la liaison série RS-232.
- Joue l’animation en demandant à l’écran d’afficher les frames une par une.
- Gère un fichier `gif_addresses.txt` comme carnet d’adresses :
  adresses des GIF déjà stockés et dernière adresse pour savoir où écrire le prochain GIF.

## Fichiers

### `WriteGif.c`

Ce qu’il fait, dans l’ordre :

- Initialise la console (clear + message de début).
- Installe des handlers de signaux :
  `SIGINT` / `SIGTERM` ⇒ arrêt propre (flag `sigFlagStop = -1`)
  `SIGUSR1` ⇒ stop lecture animation (flag `sigUser1 = 1`)
- Ouvre l’UART OLED sur `/dev/ttyAL0`.

Configure la liaison série :

- Démarre en 9600, puis envoie une commande à l’écran pour passer en 115200 et met à jour le termios côté Linux (`init_liaison` + `change_baudrate`).
- Initialise le "media" côté écran (`init_media`, commande `0xFF 0xB1`, attend un ACK `0x06` + une valeur 16-bit).

Charge `gif_addresses.txt` :

- `EnsureGifAddressFile()` ⇒ crée automatiquement le fichier s’il n’existe pas (avec un contenu minimal).
- `LoadGifAddresses()` ⇒ charge la liste d’adresses "GIF existants".
- `LoadGifFreeAddress()` ⇒ charge la dernière entrée comme adresse libre (cmd Set Byte Address, opcode `0xFF 0xB9`).
- Demande un nom de fichier à l’utilisateur.
- Si le nom commence par `c` ⇒ mode "pas d’upload" (le code positionne `NoGif = 1`), et la lecture des frames se base sur la dernière adresse GIF existante (nécessite au moins un GIF enregistré).

Sinon :

- Positionne l’adresse d’écriture sur l’adresse libre (envoi du `cmd_SetByteAdresse_Free`).
- Convertit le GIF en `.raw` avec en-tête via `build_raw_with_header()`.

Écrit le fichier sur le media :

- Lecture du fichier par blocs de 512 (`BYTES_PER_SECTOR`).
- Écriture en mots 16-bit avec `Write_Word_media()` (commande `0xFF 0xB4`).
- Barre de progression `print_progress()`.
- Flush media (`cmd_FlushMedia = 0xFF 0xB2`).
- Met à jour `gif_addresses.txt` en ajoutant la prochaine adresse libre calculée à partir de la taille du raw (alignée sur 512) : `AppendGifAddressFromRaw()`.

Récupère le nombre de frames (`GetNbFrames`) :

- Se place à l’offset `+0x07` dans l’en-tête (champ "frames", 16-bit) via Set Byte Address.
- Lit 2 octets via Read Byte (`0xFF 0xB7`) avec ACK + valeur.
- Revient à l’offset `0x00`.

Joue l’animation :

- Boucle de `i=0` à `i<=frame`.
- Envoie Display Video Frame.
- Délai `usleep(30000)`.

Nettoyage :

- Clear écran.
- Repasse en 9600.
- Restaure l’ancien termios, ferme le fd.
- Lance `./oled` (si pas interrompu par `SIGINT` / `SIGTERM`).

### `function.c`

Comms :

- `read_exact()` : lit exactement N octets (gère `EINTR`), évite les "lectures partielles" en série.
- `SendCmd()` : envoie un buffer + lit 1 octet de retour (ACK / statut).
- `SendCmd2RetV()` : envoie un buffer + lit ACK + lit une valeur 16-bit.

UART :

- `init_liaison()` : met le port en raw mode, démarre à 9600, puis appelle `change_baudrate()` pour passer à 115200.
- `change_baudrate()` : envoie la commande écran de changement de débit + modifie le débit côté Linux.

Media :

- `init_media()` : commande `0xFF 0xB1`, attend ACK `0x06` + valeur.
- `Write_Word_media()` / `Write_Byte_media()` : écriture dans le media via `0xFF 0xB4`.
- `GetNbFrames()` : lit le champ "frames" à l’offset `0x07`.

Gestion du fichier d’adresses :

- `EnsureGifAddressFile()` : si le fichier n’existe pas, le crée avec le contenu minimal (1 entrée = adresse libre).
- `LoadGifAddresses()` : lit le nombre d’entrées puis charge les entrées (sauf la dernière) sous forme de buffers Set Byte Address (6 octets).
- `LoadGifFreeAddress()` : lit la dernière entrée (zone libre).
- `AppendGifAddressFromRaw()` : calcule `next = last + taille_raw` aligné sur 512 (`(next + 0x1FF) & ~0x1FF`), réécrit le fichier avec une entrée de plus.

Conversion GIF → RAW OLED :

- `parse_gif_info()` : utilise `ffprobe` pour récupérer largeur/hauteur, nb_frames, et une durée de frame (`pkt_duration_time`) pour dériver un délai en centisecondes (borné `1..255`).
- `convert_gif_to_rgb565be()` : utilise `ffmpeg` pour produire un flux `rgb565be` en `128x128` (scaling neighbor).
- `build_raw_with_header()` : génère un fichier final : écrit un en-tête 8 octets (`width(2) height(2) bpp(1) delay(1) frames(2)`) puis concatène les données RGB565BE.

### `WriteGif.h`

Header commun :

- Includes système.
- Constantes : `GIF_ADDRESS_PATH`, `OLED_WIDTH/HEIGHT/BPP`, tailles de blocs.
- Structures : `GifInfo`, `TCharDevice`, `TDeviceCharGpIO`.
- Prototypes des fonctions.

## Format attendu de `gif_addresses.txt`

- 1ère ligne : un entier `N` (nombre total d’entrées).
- Puis `N` lignes, chacune contenant 6 octets hex sans séparateurs (lecture `%2hhX` répétée 6 fois).
- Exemple de forme : `FFB9XXXXXXXX` (6 octets = 12 hex chars).
- Interprétation :
  - Les `N−1` premières entrées = adresses des GIF stockés (Set Byte Address).
  - La dernière entrée = adresse libre (prochaine zone d’écriture).

Exemple minimal (auto-créé si absent) :

```
1
FFB902000000
```

Exemple de fichier :

```
2
FFB920322600
FFB9203E3200 <-- Prochaine adresse libre
```

On en déduit qu'il y a un GIF déjà upload sur le media et que la prochaine adresse libre est : `203E3200` en hex.

## Outils

- `ffprobe` (pour métadonnées GIF).
- `ffmpeg` (conversion en `rgb565be`).

## Utilisation (principe)

1. Lancer `./WriteGif`.
2. Entrer le nom d’un fichier GIF (ou un nom commençant par `c` pour éviter l’upload et juste jouer depuis une adresse existante).
3. Le programme upload (si mode normal), flush, met à jour `gif_addresses.txt`.
4. Puis lit `nb_frames` et joue l’animation frame par frame.

