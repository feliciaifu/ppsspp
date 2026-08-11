# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and test commands

### Windows
- Main solution: `Windows/PPSSPP.sln`
- CI build command for Win64:
  - `msbuild /m /p:TrackFileAccess=false /p:Configuration=Release /p:Platform=x64 Windows/PPSSPP.sln`
- UWP solution: `UWP/PPSSPP_UWP.sln`

### CMake presets
- `cmake --preset gcc-debug`
- `cmake --build build-gcc-debug`
- `cmake --preset clang-debug`
- `cmake --build build-clang-debug`

### Common native builds
- `./b.sh --headless --unittest`
- `./b.sh --qt`
- `./b.sh --release`
- `./b.sh --debug`

`b.sh` is a convenience wrapper around CMake + make. Useful flags verified in the script include `--headless`, `--unittest`, `--qt`, `--ios`, `--release`, `--debug`, `--reldebug`, `--sanitize`, and `--sanitizeub`.

### Unit tests
- Build with unittest enabled, then run:
  - `./PPSSPPUnitTest ALL`
- Run a single test by name:
  - `./PPSSPPUnitTest VertexJit`

`CMakeLists.txt` also registers individual ctest entries, so from a configured build directory you can run targeted tests such as:
- `ctest -R vertex_jit`

### Headless / pspautotests
- CI uses:
  - `python test.py -g --graphics=software`

This requires a headless build and the `pspautotests` content to be present. `test.py` can also run a single autotest path when needed.

### Android
- Modern Android builds use Gradle + externalNativeBuild(CMake).
- Typical command:
  - `./gradlew assembleNormalDebug`
- Other common variants in the Gradle config / CI include optimized, release, gold, vr, and legacy flavors.

Note: `android/README.TXT` describes an older `ab.sh` / `ab.cmd` ndk-build flow. CI still uses it in some jobs, but it is marked legacy in the repo.

## High-level architecture

### Main layers
- `Common/`: cross-platform infrastructure, utility code, file handling, threading, rendering abstractions, networking, profiling, and shared helpers.
- `Core/`: PSP emulation core, loaders, HLE modules, memory, filesystem emulation, debugger support, MIPS CPU/JIT/IR, and system lifecycle.
- `GPU/`: shared GPU logic plus backend-specific implementations (`Vulkan`, `GLES`, `D3D11`, `Directx9`, `Software`).
- `UI/`: shared frontend screens and workflows, including main menu, game browser, settings, pause flow, and runtime UI.
- Platform frontends: `Windows/`, `SDL/`, `Qt/`, `android/`, `ios/`, `UWP/`.
- Special targets: `headless/`, `unittest/`, `libretro/`.

### Boot / launch flow
- `UI/MainScreen.cpp`
  - Main menu and game browser entry points.
  - `LaunchFile(ScreenManager *, const Path &)` is the central UI-side launcher.
- `UI/EmuScreen.cpp`
  - `EmuScreen::ProcessGameBoot()` converts a selected path into a `CoreParameter` boot request and calls `PSP_InitStart()`.
- `Core/System.cpp`
  - `PSP_InitStart()` / `PSP_InitUpdate()` manage the async boot lifecycle.
  - Boot resolves a file loader, identifies file type, and initializes CPU / system state.
- `Core/Loaders.cpp`
  - File type identification for bootable content.

If a task changes how something starts, inspect these files first.

### Filesystem mapping and runtime directories
- `Core/HLE/sceIo.cpp` mounts PSP-visible devices such as `ms0:` and `flash0:`.
- `g_Config.memStickDirectory` backs the emulated memory stick.
- `g_Config.flash0Directory` backs `flash0:`.
- `Windows/main.cpp`, `UI/NativeApp.cpp`, `headless/Headless.cpp`, and some other platform entry points initialize these directories differently per platform.

This is important for any feature that depends on firmware-like resources or PSP-visible paths.

### Platform entry points
- `Windows/main.cpp`: Win32 frontend entry and Windows-specific initialization.
- `SDL/SDLMain.cpp`: SDL-based desktop frontend.
- `Qt/QtMain.cpp`: Qt frontend entry.
- `android/jni/app-android.cpp`: JNI bridge and Android native entry.
- `UI/NativeApp.cpp`: shared native app initialization path used by multiple non-Windows frontends.
- `headless/Headless.cpp`: non-UI execution path used for automated tests.

## Repository-specific facts that matter
- The repo does not currently include a complete PSP firmware tree in `assets/flash0`; the checked-in `assets/flash0` content is primarily fonts.
- Features that expect full `flash0:` contents must verify that the runtime environment provides the needed files through `g_Config.flash0Directory`.
- Root `README.md` is product-oriented. For development workflow, the most reliable sources are `b.sh`, `CMakeLists.txt`, `CMakePresets.json`, `android/build.gradle`, and `.github/workflows/build.yml`.
- There is no existing repo-local `CLAUDE.md`, `.cursorrules`, `.cursor/rules/`, or `.github/copilot-instructions.md` to inherit behavior from.

## Jpcsp
Jpcsp是一个使用Java'编写的psp模拟器，它可以启动psp的vsh，显示xmb主界面并从xmb启动游戏，在实现ppsspp的vsh启动时可以参考，
Jpcsp项目位于D:\repos\jpcsp-master