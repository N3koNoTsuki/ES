# README — Installation de FFmpeg (conversion GIF → RGB565BE) sur cible DE1-SoC

Ce document décrit, pas à pas, comment intégrer FFmpeg dans un rootfs Linux
embarqué pour DE1-SoC, afin de permettre la conversion de GIF vers du
rawvideo RGB565 big-endian (rgb565be).

Chemins utilisés :

- Rootfs cible : ~/DE1-Soc/rootfs/
- Staging FFmpeg : ~/DE1-Soc/ffmpeg-armhf/

Objectif final :

- /usr/bin/ffmpeg disponible sur la cible
- Commande fonctionnelle :
  `ffmpeg -i input.gif -f rawvideo -pix_fmt rgb565be output.rgb`


## 1) Prérequis

### 1.1 Architecture cible

- ARMv7-A (Cortex-A9)
- ABI : armhf (hard-float)
- Loader présent :
  `~/DE1-Soc/rootfs/lib/ld-linux-armhf.so.3`

### 1.2 Outils nécessaires sur le PC hôte (Debian / Ubuntu)

- gcc-arm-linux-gnueabihf
- make
- pkg-config
- yasm / nasm
- git

Installation :

```bash
sudo apt update
sudo apt install -y \
  gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
  make pkg-config yasm nasm git
```

## 2) Récupération des sources FFmpeg

```bash
mkdir -p ~/src
cd ~/src
git clone https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg
```
	
## 3) Compilation de FFmpeg pour ARM (cross-compilation)

### 3.1 Dossier d’installation (staging)

```bash
mkdir -p ~/DE1-Soc/ffmpeg-armhf
```
	
### 3.2 Configuration + build

```bash
export TARGET=arm-linux-gnueabihf
export PREFIX=$HOME/DE1-Soc/ffmpeg-armhf

make distclean 2>/dev/null || true

./configure \
  --prefix="$PREFIX" \
  --arch=arm \
  --cpu=cortex-a9 \
  --target-os=linux \
  --cross-prefix=${TARGET}- \
  --enable-cross-compile \
  --enable-gpl \
  --disable-debug \
  --disable-doc \
  --disable-ffplay \
  --enable-ffmpeg \
  --enable-ffprobe \
  --enable-small \
  --disable-xlib \
  --disable-sdl2 \
  --disable-indevs \
  --disable-outdevs \
  --enable-protocol=file \
  --enable-demuxer=rawvideo \
  --enable-muxer=rawvideo \
  --enable-decoder=rawvideo \
  --enable-encoder=rawvideo \
  --enable-parser=rawvideo

make -j$(nproc)
make install
```

## 4) Vérification du binaire FFmpeg

```bash
file ~/DE1-Soc/ffmpeg-armhf/bin/ffmpeg
```
	
Attendu :

- ELF 32-bit
- ARM
- EABI5
- linked dynamiquement ou statiquement (les deux sont valides)


## 5) Copie de FFmpeg dans le rootfs

IMPORTANT :
Si un dossier est créé avec sudo, la copie doit aussi se faire avec sudo
(ou bien corriger les permissions).

```bash
sudo mkdir -p ~/DE1-Soc/rootfs/usr/bin

sudo cp -av ~/DE1-Soc/ffmpeg-armhf/bin/ffmpeg \
  ~/DE1-Soc/rootfs/usr/bin/

sudo cp -av ~/DE1-Soc/ffmpeg-armhf/bin/ffprobe \
  ~/DE1-Soc/rootfs/usr/bin/ 2>/dev/null || true

sudo chmod +x ~/DE1-Soc/rootfs/usr/bin/ffmpeg
sudo chmod +x ~/DE1-Soc/rootfs/usr/bin/ffprobe 2>/dev/null || true
```

## 6) Dépendances runtime

### 6.1 Vérification des dépendances (sur le PC)

```bash
arm-linux-gnueabihf-readelf -d \
  ~/DE1-Soc/rootfs/usr/bin/ffmpeg | grep NEEDED
```

Dans ce cas précis, FFmpeg dépend uniquement de :

- libc.so.6
- libm.so.6
- ld-linux-armhf.so.3

### 6.2 Vérifier leur présence dans le rootfs

```bash
ls -l ~/DE1-Soc/rootfs/lib/ld-linux-armhf.so.3
find ~/DE1-Soc/rootfs/lib -name 'libc.so.6' -o -name 'libm.so.6'
```

### 6.3 Copier libc/libm si absentes

```bash
SYSROOT=$(arm-linux-gnueabihf-gcc -print-sysroot)

sudo mkdir -p ~/DE1-Soc/rootfs/lib/arm-linux-gnueabihf

sudo cp -av "$SYSROOT/lib/arm-linux-gnueabihf/libc.so.6" \
  ~/DE1-Soc/rootfs/lib/arm-linux-gnueabihf/

sudo cp -av "$SYSROOT/lib/arm-linux-gnueabihf/libm.so.6" \
  ~/DE1-Soc/rootfs/lib/arm-linux-gnueabihf/

# (Si nécessaire)
sudo cp -av "$SYSROOT/lib/ld-linux-armhf.so.3" \
  ~/DE1-Soc/rootfs/lib/
```

## 7) Déploiement du rootfs sur la cible

Selon le workflow utilisé :

### 7.1 Par rsync (cible accessible en SSH)

```bash
rsync -av --delete ~/DE1-Soc/rootfs/ root@IP_DE_LA_CIBLE:/
```

### 7.2 Via une image ext4 montée en loop

```bash
mkdir -p /tmp/mnt-rootfs
sudo mount -o loop rootfs.img /tmp/mnt-rootfs
sudo rsync -av --delete ~/DE1-Soc/rootfs/ /tmp/mnt-rootfs/
sudo umount /tmp/mnt-rootfs
```
		
## 8) Test sur la cible DE1-SoC

Sur la cible :

```bash
/usr/bin/ffmpeg -version
```

Cas d’erreurs courantes :

- "No such file or directory" :
  → loader manquant ou mauvais ABI
- "error while loading shared libraries" :
  → librairie absente dans le rootfs


## 9) Checklist finale

- [ ] /usr/bin/ffmpeg présent dans le rootfs
- [ ] /lib/ld-linux-armhf.so.3 présent
- [ ] libc.so.6 et libm.so.6 présents
- [ ] ffmpeg -version fonctionne sur la cible
- [ ] conversion GIF → RGB565BE validée

