## Txtquad
- A math library and Vulkan graphics pipeline to render textured quads in 3D, for Linux and Windows.
- A complete rewrite and extension of my best friend's [eponymous engine](https://github.com/acgaudette/txtquad) for game jams.

## Dependencies
- [GLFW 3.3](https://github.com/glfw/glfw/releases/tag/3.3.4) (GLFW headers and library).
- [Vulkan 1.2](https://vulkan.lunarg.com/sdk/home) (Vulkan loader, headers, validation layer, and supported driver).
- [Shaderc](https://github.com/google/shaderc) (`glslc`, included with LunarG Vulkan SDK)

## [`build.ninja`](build.ninja)
- You must have [Ninja](https://github.com/ninja-build/ninja/releases) installed.
- There are build targets for Linux and Windows hosts.
- Before building, create the directory symlinks `glfw` and `vulkan`
  - These symlinks point to the installation directories containing the headers and libraries.
  - Windows PowerShell (Run as Administrator)
    - `New-Item -Path glfw   -ItemType SymbolicLink -Value C:\path\to\glfw`
    - `New-Item -Path vulkan -ItemType SymbolicLink -Value C:\path\to\vulkan`
  - Linux Shell
    - `ln -s /path/to/glfw   glfw  ` (hint: `ln -s /usr glfw  ` to use system installation)
    - `ln -s /path/to/vulkan vulkan` (hint: `ln -s /usr vulkan` to use system installation)
- Build the font editor and demo for your host platform
  - Windows `ninja win`
  - Linux   `ninja lin` (default)
- Or build just the library for your host platform.
  - Windows `ninja txtquad.lib`
  - Linux   `ninja libtxtquad.so`
- On Linux, the build uses `libglfw.so` by default
  - You can switch to `libglfw3.a` by changing `-lglfw` to `-lglfw3`
- Clang is used as a cross-platform C compiler. You can modify `rule c` and `rule l` to configure a different compiler and linker.

## [`txtquad.h`](txtquad.h)
- `txt.quad` points to the GPU-mapped quad buffer
  - It should contain `txt.quads` quads to be rendered.
- `txt.viewproj` points to the GPU-mapped view projection matrix.
- define `Input` before including to enable input.
- define `InputUtil` before including to enable the input utility, which automatically tracks inputs to poll based on which Key and Mouse macros you use.

## [`input.h`](input.h)
- Has keyboard key and mouse button state polling.
- Has callbacks for unicode, mouse cursor position, and mouse scroll.
- Consult [`inputdefines.h`](inputdefines.h) for the available key and button names.

## [`alg.h`](alg.h)
- Math stuff (a technical term).
- Read [`font.c`](font.c) or [`demo.c`](demo.c) to see the basics in action.
- Tries its best to abuse the C `_Generic` keyword.
- None of this is optimized.

## [`font.c`](font.c)
- A bitmap editor to edit the PBM textures for the engine (try `./font font.pbm`).

## Windows
- To install the C/C++ toolchain on Windows 10
  - **EITHER** Install [Visual Studio Community 2022](https://visualstudio.microsoft.com/downloads/#visual-studio-community-2022)
  - **OR** Install [Visual Studio Build Tools 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
  - I also built this with Visual Studio 2019 previously.
- Once the installation is complete, click the "Modify" button next to your installation, then click the "Individual Components" tab
  - Search for "MSVC v143 - VS 2022 C++" and select an appropriate version for your target platform (Latest is fine)
  - Search for "Windows 10 SDK" and select an appropriate version for your target platform (if in doubt pick the highest version number)
- To install Ninja
  - **EITHER** Select "C++ CMake tools for Windows" in "Individual Components" in the Visual Studio Installer (CMake comes with Ninja, obviously /s)
  - **OR** Download the Windows release of Ninja (usually `ninja-win.zip`) from the [Ninja GitHub Releases](https://github.com/ninja-build/ninja/releases), unzip it and make sure `ninja.exe` can be found in your Windows `Path` environment variable.
- To install Clang
  - **EITHER** Select "C++ Clang Compiler for Windows" in "Individual Components" in the Visual Studio Installer
  - **OR** Try your luck with a Windows release `.exe` from the [LLVM GitHub Releases](https://github.com/llvm/llvm-project/releases). My personal impression is that LLVM keeps messing with their Windows release packaging, and that their shit seems unstable and inadequately tested on Windows. The Visual Studio release configuration is *probably* significantly different, and *possibly* significantly more appropriate.
- To use the toolchain
  - **EITHER** Launch a "Native Tools Command Prompt for VS 2022" shortcut for your target architecture, for example x86 or x64. From that command prompt you can launch PowerShell by running `pwsh` if you prefer.
  - **OR** add the required executable paths to your `Path` user or system environment variable, and the required linker paths to your `LIBPATH` environment variable which is expected by `lib.exe` and normally defined by `vcvarsall.bat`. Check that your `Path` and `LIBPATH` are actually correct for your target architecture, since for example on x64, Microsoft distributes its toolchain as a x86/x64 cross-target toolchain.
