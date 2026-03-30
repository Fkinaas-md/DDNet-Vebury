# DDNet 16.1 Hash Patch

<p align="center">
  <b>Patched DDNet 16.1 build for modern Linux distributions</b>
</p>

---

## About

This repository contains a patched DDNet 16.1 source tree with fixes for building on newer toolchains and Linux distributions.

**Key change**: Added hash patch to client version string for full server compatibility while maintaining DDNet 16.1 source base.

## Features

- DDNet 16.1 source base.
- Compatibility fixes for modern GCC/Clang versions.
- Correct client hash for server acceptance.
- Ready-to-build project layout.
- Useful for Fedora, Void Linux, Arch, Debian, Ubuntu, and other modern distributions.

## Supported distributions

- Fedora
- Arch Linux
- Debian / Ubuntu
- Void Linux
- openSUSE
- Other modern Linux distributions

## Dependencies

### Fedora

```bash
sudo dnf install -y \
  git cmake ninja-build gcc-c++ make \
  freetype-devel png-devel zlib-devel \
  openal-soft-devel opusfile-devel \
  libvorbis-devel curl-devel \
  SDL2-devel
```

### Arch Linux

```bash
sudo pacman -S --needed \
  git cmake ninja gcc make \
  freetype2 libpng zlib \
  openal-soft opusfile libvorbis curl \
  sdl2
```

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install -y \
  git cmake ninja-build g++ make \
  libfreetype6-dev libpng-dev zlib1g-dev \
  libopenal-dev libopusfile-dev \
  libvorbis-dev libcurl4-openssl-dev \
  libsdl2-dev
```

### Void Linux

```bash
sudo xbps-install -Syu
sudo xbps-install -S \
  git cmake ninja gcc make \
  freetype-devel libpng-devel zlib-devel \
  openal-soft-devel opusfile-devel \
  libvorbis-devel curl-devel \
  SDL2-devel
```

### openSUSE

```bash
sudo zypper install -y \
  git cmake ninja gcc-c++ make \
  freetype2-devel libpng16-devel zlib-devel \
  openal-soft-devel opusfile-devel \
  libvorbis-devel libcurl-devel \
  SDL2-devel
```

## Clone repository + build

```bash
git clone https://github.com/Fkinaas-md/DDNet-16.1-Hash
cd DDNet-16.1-Hash
git submodule update --init --recursive
mkdir -p build
cd build
cmake -GNinja -DVIDEORECORDER=OFF ..
ninja -j$(nproc)
```

## Notes

- If the build fails because of missing headers or libraries, install the matching `-dev` or `-devel` package for your distribution.
- Some systems may also need additional audio, window system, or graphics packages depending on the desktop environment.
- For Void Linux, make sure `xbps-install -Syu` is run before installing dependencies.

## License

This project follows the original DDNet licensing terms.

## Acknowledgements

- DDNet project contributors.
- Linux distribution maintainers.
