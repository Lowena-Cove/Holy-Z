# Building Holy-Z

Holy-Z requires several dependencies to compile: Boost and SDL2 libraries.

## Option 1: Using vcpkg (Recommended)

vcpkg is a C++ package manager that makes it easy to manage dependencies.

### Setup Steps:

1. **Install vcpkg** (if not already installed):
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. **Build Holy-Z with vcpkg integration**:
   ```powershell
   cd <path-to-Holy-Z>/HolyZ
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE='<vcpkg-root>/scripts/buildsystems/vcpkg.cmake'
   cmake --build build --config Release
   ```

   Replace `<vcpkg-root>` with your vcpkg installation directory.

## Option 2: Manual Installation

If you prefer to install dependencies manually:

### Windows:

1. **Install Boost**:
   - Download from: https://sourceforge.net/projects/boost/files/
   - Or use a package manager like `vcpkg` or `chocolatey`
   - Add the Boost include and lib paths to the commented sections in `CMakeLists.txt`

2. **Install SDL2, SDL2_image, and SDL2_ttf**:
   - Download from: https://www.libsdl.org/
   - Or use vcpkg: `vcpkg install sdl2:x64-windows sdl2-image:x64-windows sdl2-ttf:x64-windows`

3. **Build**:
   ```powershell
   cd HolyZ
   cmake -B build -S .
   cmake --build build --config Release
   ```

### Linux:

1. **Install dependencies using package manager**:
   ```bash
   # Debian/Ubuntu
   sudo apt-get install libboost-dev libboost-system-dev libboost-filesystem-dev \
                        libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev

   # Arch Linux
   sudo pacman -S boost sdl2 sdl2_image sdl2_ttf
   ```

2. **Build**:
   ```bash
   cd HolyZ
   cmake -B build -S .
   cmake --build build
   ```

## Build Output

The compiled executable will be located in:
- Windows: `HolyZ/build/Release/HolyZ.exe` or `HolyZ/build/Debug/HolyZ.exe`
- Linux: `HolyZ/build/HolyZ`

## Troubleshooting

- **"Boost not found"**: Make sure Boost is installed and CMake can find it. Use vcpkg for the easiest setup.
- **"SDL2 not found"**: Graphics features will be disabled, but the interpreter will still compile. Install SDL2 if you need graphics support.
- **Compilation errors**: Make sure you're using C++17 or later and have all required headers installed.
