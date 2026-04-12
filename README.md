<h1 align="center">DDNet 16.1 Hash Patch</h1>

<p align="center">
  <b>Patched DDNet 16.1 build for modern Linux distributions</b>
</p>

---

## About

This repository contains a patched DDNet 16.1 source tree with fixes for building on newer toolchains and Linux distributions.

**Key change**: Added hash patch to client version string for full server compatibility while maintaining DDNet 16.1 source base.

> **Note**: I prefer source code builds over precompiled ZIP files, so no binaries will be provided here.

## Features

- DDNet 16.1 source base.
- Compatibility fixes for modern GCC/Clang versions.
- Correct client hash for server acceptance.
- Ready-to-build project layout.
- Useful for Fedora, Void, Arch, Debian, Ubuntu, Gentoo, and other modern distributions.

## Supported distributions

- Fedora
- Arch Linux
- Debian / Ubuntu
- Void Linux
- Gentoo
- Other modern Linux distributions

## Dependencies

### Fedora

```bash
sudo dnf install
cargo cmake ffmpeg-devel freetype-devel gcc gcc-c++ git glew-devel
glslang gmock-devel gtest-devel libogg-devel libpng-devel
libcurl-devel libnotify-devel make ninja-build openssl-devel
opus-devel opusfile-devel python SDL2-devel x264-devel
spirv-tools sqlite-devel vulkan-devel wavpack-devel
```

### Arch

```bash
sudo pacman -S --needed base-devel cmake curl ffmpeg freetype2 git glew glslang gmock
libnotify libpng ninja opusfile python rust sdl2 spirv-tools sqlite vulkan-headers
vulkan-icd-loader wavpack x264
```

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install
build-essential cargo cmake git glslang-tools google-mock libavcodec-extra
libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev libcurl4-openssl-dev
libfreetype6-dev libglew-dev libnotify-dev libogg-dev libopus-dev libopusfile-dev
libpng-dev libsdl2-dev libsqlite3-dev libssl-dev libvulkan-dev libwavpack-dev libx264-dev
ninja-build python3 rustc spirv-tools
```

### Void

```bash
sudo xbps-install -Syu
sudo xbps-install -S base-devel cargo cmake ffmpeg6-devel freetype-devel git glew-devel
glslang gtest-devel libcurl-devel libnotify-devel libogg-devel libpng-devel ninja
openssl-devel opus-devel opusfile-devel sqlite-devel SPIRV-Tools-devel vulkan-loader
wavpack-devel x264-devel SDL2-devel
```

### Gentoo 

```bash
emerge --ask dev-build/ninja dev-db/sqlite dev-lang/rust-bin dev-libs/glib
dev-libs/openssl dev-util/glslang dev-util/spirv-headers dev-util/spirv-tools
media-libs/freetype media-libs/glew media-libs/libglvnd media-libs/libogg
media-libs/libpng media-libs/libsdl2 media-libs/libsdl2[vulkan] media-libs/opus
media-libs/opusfile media-libs/pnglite media-libs/vulkan-loader[layers]
media-sound/wavpack media-video/ffmpeg net-misc/curl x11-libs/gdk-pixbuf x11-libs/libnotify
```
## Clone repository + build

```bash
git clone https://github.com/Fkinaas-md/DDNet-16.1-Hash
cd DDNet-16.1-Hash
git submodule update --init --recursive
cmake -Bbuild -GNinja -DVIDEORECORDER=OFF 
cmake --build build
```

## Notes

- If the build fails because of missing headers or libraries, install the matching `-dev` or `-devel` package for your distribution.
- Some systems may also need additional audio, window system, or graphics packages depending on the desktop environment.

## License

This project follows the original DDNet licensing terms.

## Acknowledgements

- DDNet project contributors.
- Linux distribution maintainers.
