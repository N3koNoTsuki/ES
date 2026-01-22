# README — ./DIsplayGif (sélection et lecture d’animations GIF sur écran OLED via boutons GPIO)

- je recommande d'utiliser l'extension `Simply Markdown Viewer` sous Visual pour un meilleur affichage ou tout viewer de .md


## Résumé

`./DIsplayGif` est un programme Linux (cible ARM) qui :

- Lit l’état de boutons physiques via des GPIO Linux.
- Sélectionne un GIF déjà stocké dans la mémoire media de l’écran OLED.
- Pilote l’écran OLED via RS-232 pour :
  - Positionner l’adresse mémoire du GIF.
  - Lire le nombre de frames.
  - Afficher les frames une par une.
- Permet de changer d’animation à la volée lors d’un appui sur un bouton.

## Fichiers

### `DIsplayGif.c`

Ce qu’il fait, dans l’ordre :

- Initialisation générale.
- Initialise la console.

Installe les handlers de signaux :

- `SIGINT` / `SIGTERM` ⇒ arrêt global.
- `SIGUSR1` ⇒ interruption de lecture animation / changement de GIF.
- Crée un pipe anonyme pour communiquer entre deux processus.
- Effectue un `fork()`.

Processus fils — gestion des boutons GPIO :

- Configure les GPIO boutons via `config_gpio_button()`.
- Boucle tant que `sigFlagStop != -1` :
  - Détecte un appui bouton via `check_all_buttons()`.
  - Écrit l’index du bouton dans le pipe.
  - Envoie `SIGUSR1` au processus père.
- À l’arrêt :
  - Envoie `SIGINT` au père.
  - Ferme les GPIO et le pipe.

Processus père — pilotage OLED + lecture animation :

- Charge `gif_addresses.txt` via `LoadGifAddresses()` :
  - Récupère la liste des adresses GIF jouables.
  - Ouvre l’UART OLED sur `/dev/ttyAL0`.
- Initialise la liaison série :
  - Démarre en 9600.
  - Bascule en 115200 via `init_liaison()` / `change_baudrate()`.
  - Initialise le media OLED.
  - Sélectionne un GIF courant.
  - Lit son nombre de frames via `GetNbFrames()`.
  - Boucle principale.

Tant que `sigFlagStop != -1` :

- Si un bouton a été pressé (`sigUser1 == 1`) :
  - Lit l’index bouton depuis le pipe.
  - Calcule le nouvel index GIF via `GstPipe()`.
  - Recharge le nombre de frames avec `GetNbFrames()`.
  - Remet `sigUser1 = 0`.
- Lecture de l’animation :
  - Boucle `i = 0` → `nb_frames`.
  - Envoie Display Video Frame.
  - Délai `usleep(30000)`.
  - Interruption immédiate si un bouton est pressé.

Nettoyage :

- Libère la table des adresses GIF.
- Efface l’écran OLED.
- Repasse l’UART en 9600.
- Restaure le termios.
- Ferme `/dev/ttyAL0`.
- Attend la fin du processus fils.

### `function.c`

Comms :

- `read_exact()` : lit exactement N octets (gestion d’`EINTR`), évite les lectures partielles.
- `SendCmd()` : envoie une commande et lit un octet de retour (ACK / statut).
- `SendCmd2RetV()` : envoie une commande, lit l’ACK puis une valeur 16-bit.

UART :

- `init_liaison()` : met le port en mode raw, démarre en 9600, puis passe en 115200.
- `change_baudrate()` : envoie la commande OLED de changement de débit et met à jour le termios.

Media OLED :

- `init_media()` : initialise la mémoire media (`0xFF 0xB1`, ACK `0x06`).
- `Write_Word_media()` / `Write_Byte_media()` : écriture dans le media (`0xFF 0xB4`).
- `GetNbFrames()` : lit le champ frames à l’offset `0x07` de l’en-tête GIF.

Gestion des adresses :

- `LoadGifAddresses()` : lit le nombre d’entrées et charge les `N−1` premières adresses (GIF jouables).

### `DIsplayGif.h`

Header commun :

- Includes système.
- Constantes : `GIF_ADDRESS_PATH`, `OLED`, tailles de buffers.
- Structures : `TCharDevice`, `TDeviceCharGpIO`.
- Prototypes des fonctions GPIO, UART et media.

## Format attendu de `gif_addresses.txt`

- 1ère ligne : un entier `N` (nombre total d’entrées).
- Puis `N` lignes, chacune contenant 6 octets hex sans séparateurs (lecture `%2hhX` répétée 6 fois).
- Exemple de forme : `FFB9XXXXXXXX` (6 octets = 12 hex chars).
- Interprétation :
  - Les `N−1` premières entrées = adresses des GIF stockés (Set Byte Address).
  - La dernière entrée = adresse libre (prochaine zone d’écriture).

Exemple de fichier :

```
2
FFB920322600
FFB9203E3200 <-- Prochaine adresse libre
```

On en déduit qu'il y a un GIF déjà upload sur le media et que la prochaine adresse libre est : `203E3200` en hex.

## Utilisation (principe)

1. Lancer `./DIsplayGif`.
2. L’écran affiche l’animation du GIF courant.
3. Appuyer sur un bouton :
   - Arrêt immédiat de l’animation.
   - Sélection d’un autre GIF.
   - Reprise de la lecture.
4. `Ctrl+C` ⇒ arrêt propre.