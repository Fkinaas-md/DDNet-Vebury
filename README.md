<h1 align="center">Vebury</h1>

<p align="center">
  <b>DDNet 16.1 with patches</b>
</p>

---

## About
DDraceNetwork 16.1 with my patches :p

## Modifications

- **Vebury RCON tab** — Mass kick/ban UI with player list, multi-select, duration & reason fields.
- **Chat text selection** — Shift+arrows, Ctrl+A, highlight, replace/delete selection.
- **Console text selection** — Same as chat, different highlight colors for local/remote F1/F2.
- **Unified `clear`** — Replaces `clear_local_console` / `clear_remote_console`.
- **Scoreboard cursor** — `toggle_scoreboard_cursor` command + `cl_scoreboard_cursor` config.
- **FFmpeg 7.x compat** — Video recording works without `-DVIDEORECORDER=OFF`. Backward compatible.
- **Checksum fixes** — GCC15 support, executable caching, negative length fix.
- **Build system** — CMake 2.8.12+, GCC15, Emscripten support.

## Build

```bash
git clone https://github.com/fkinaas-md/DDNet-Vebury
cd DDNet-Vebury
cmake -Bbuild -GNinja
cmake --build build
```

## Dependencies
### Debian

```bash
sudo apt update
sudo apt install
build-essential cargo cmake git glslang-tools google-mock libavcodec-extra
libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev libcurl4-openssl-dev
libfreetype6-dev libglew-dev libnotify-dev libogg-dev libopus-dev libopusfile-dev
libpng-dev libsdl2-dev libsqlite3-dev libssl-dev libvulkan-dev libwavpack-dev libx264-dev
ninja-build python3 rustc spirv-tools
```

## License

This project follows the original DDNet licensing terms.

