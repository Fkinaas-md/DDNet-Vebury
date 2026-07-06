<h1 align="center">DDNet 16.1 — Modded Client</h1>

<p align="center">
  <b>Patched DDNet 16.1 build with UI improvements and modern toolchain fixes</b>
</p>

---

## Modifications

- **Vebury RCON tab** — Mass kick/ban UI with player list, multi-select, duration & reason fields.
- **Chat text selection** — Shift+arrows, Ctrl+A, highlight, replace/delete selection.
- **Console text selection** — Same as chat, different highlight colors for local/remote F1/F2.
- **Unified `clear`** — Replaces `clear_local_console` / `clear_remote_console`.
- **Scoreboard cursor** — `toggle_scoreboard_cursor` command + `cl_scoreboard_cursor` config.
- **FFmpeg 7.x compat** — Video recording works without `-DVIDEORECORDER=OFF`. Backward compatible.
- **Checksum fixes** — GCC15 support, executable caching, negative length fix.
- **Build system** — CMake 2.8.12+, GCC15, Emscripten support.
- **Pinned version** — Returns `992f40302` (DDNet 16.1).

## Build

```bash
git clone <repo-url>
cd <repo-dir>
git submodule update --init --recursive
cmake -B build -G Ninja
cmake --build build
```

## Dependencies

### Debian / Ubuntu
```
sudo apt install build-essential cargo cmake git libavcodec-dev libavformat-dev \
libavutil-dev libcurl4-openssl-dev libfreetype6-dev libglew-dev libsdl2-dev \
libsqlite3-dev libssl-dev libswscale-dev libswresample-dev libvulkan-dev ninja-build rustc
```

### Fedora
```
sudo dnf install cargo cmake ffmpeg-devel freetype-devel gcc gcc-c++ git glew-devel \
libcurl-devel make ninja-build openssl-devel SDL2-devel vulkan-devel
```

### Arch
```
sudo pacman -S --needed base-devel cmake curl ffmpeg freetype2 git glew ninja rust sdl2 vulkan-headers
```

## License

Follows original DDNet licensing terms.
