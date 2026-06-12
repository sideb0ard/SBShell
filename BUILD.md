# Building SoundB0ard

SoundB0ard is a command-line music making environment that can be built on Linux, macOS, and other Unix-like systems.

## Quick Start

0. **Download this repository and enter directory**
   ```
   # Either git clone or download zip.
   git clone git@github.com:sideb0ard/SBShell.git
   cd SBShell
   # or download zip from https://github.com/sideb0ard/SBShell
   cd ~/Downloads
   unzip SBShell-Main.zip
   cd SBShell-Main
   ```

1. **Install dependencies** (run the provided script):
   ```bash
   ./install_deps.sh
   ```

2. **Build the project**:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . -j$(nproc)
   cd ..
   ```

4. **Run**
    ```bash
    # Run from project root (so it can find wavs/ directory)
    build/Sbsh
    ```

## Manual Dependency Installation

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git \
    portaudio19-dev libportmidi-dev \
    libsndfile1-dev libreadline-dev \
    pkg-config
```

### macOS (with Homebrew)
```bash
brew install \
    portaudio portmidi libsndfile \
    readline pkg-config
```

### Arch Linux
```bash
sudo pacman -S --needed \
    base-devel cmake git \
    portaudio portmidi libsndfile \
    readline pkgconf
```

### Red Hat/CentOS/Fedora
```bash
sudo yum install -y \
    gcc-c++ cmake git \
    portaudio-devel portmidi-devel \
    libsndfile-devel readline-devel \
    pkgconfig
```

## Dependencies

SoundB0ard uses the following libraries:

### Automatically Downloaded (via FetchContent)
- **Ableton Link** - For tempo synchronization
- **PerlinNoise** - For procedural noise generation
- **nlohmann/json** - JSON serialization for preset files (uses system install if available: `brew install nlohmann-json` / `apt install nlohmann-json-dev`)

### System Libraries (must be installed)
- **PortAudio** - Cross-platform audio I/O
- **PortMidi** - Cross-platform MIDI I/O  
- **libsndfile** - Audio file reading/writing
- **Readline** - Command-line editing
- **pkg-config** - Package configuration

## IDE / Editor Support

Running `cmake ..` automatically generates `compile_commands.json` in the build directory and symlinks it to the project root. This gives clangd (used by VS Code, Neovim, CLion, etc.) accurate include paths and compiler flags, eliminating false LSP errors.

No extra steps needed — it happens as part of the normal build setup.

## Troubleshooting

- **CMake fails to find libraries**: Install the development packages for your distribution
- **Link errors**: Make sure all libraries are installed with their development headers
- **Permission denied**: Run `chmod +x install_deps.sh` to make the script executable
