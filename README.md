# Descartes Builder

![Logo](app/resources/descartes_logo.png)
[![build descartes-builder](https://github.com/CPS-research-group/descartes-builder/actions/workflows/cmake_build.yml/badge.svg)](https://github.com/CPS-research-group/descartes-builder/actions/workflows/cmake_build.yml)

DesCartes Builder is a tool to create digital twin (DT) pipelines using the Function+Data Flow (FDF) domain-specific language for machine-learning-based DT modeling. It provides three high-level features:

1. visual specification of the DT pipeline with implicit typing to detect common errors,
2. automatic generation of executable models from the FDF specification,
3. validation of design goals and ML models.

FDF defines three types of boxes to represent key steps of a DT modeling workflow:

1. Processor for typical data processing by reusing functions learned by other boxes, as well as predefined functions.
2. Coder for model order reduction by learning an encoding/decoding function with unsupervised learning algorithms (e.g., principal component analysis),
3. Trainer for data assimilation by learning a function with supervised ML (e.g., a neural network learned with gradient descent).

# Getting started

Here are instructions to use DesCartes Builder with a Docker container.

1. Load the docker image `docker-descartes-builder.tar` (docker may require `sudo` root privileges):

```bash
docker load -i docker-descartes-builder.tar
```

2. Ensure x11 is installed

```bash
sudo apt update && sudo apt install -y x11-apps xorg x11-xserver-utils
```

3. Run the container

```bash
docker run -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix descartes_builder:0.1
```

Note: You may need to use `xhost +local:docker` prior to `docker run` to authorize Docker to connect to your X server.

Details:

- `-it` makes the container reactive to your input.
- `-e` forwards $DISPLAY to the container, enabling GUI applications to display on the host's screen.
- `-v` mounts the X11 Unix socket from the host to the container.

# Build instructions 
A summary of build instructions is given below. 
# Build instructions

A summary of build instructions is given below.

## Building the Docker image

1. Clone this repository and the associated kedro-umbrella back-end

```bash
mkdir builder && cd builder
git clone --recurse-submodules git@github.com:CPS-research-group/descartes-builder.git
git clone git@github.com:eduardoconto/kedro-umbrella.git
```

2. Build the container:

```bash
docker build -t descartes_builder:0.1 -t descartes_builder:latest . -f descartes-builder/Dockerfile
```

## Building outside Docker 

### Linux Environment
## Building outside Docker

### Linux Environment

1. Clone this repository and the associated kedro-umbrella back-end

```bash
git clone --recurse-submodules git@github.com:CPS-research-group/descartes-builder.git
git clone git@github.com:eduardoconto/kedro-umbrella.git
```

2. Install the kedro-umbrella package (using a `conda` environment is highly recommended)

```bash
# Create development env
conda create -n builder python=3.10.8 && conda activate builder
# Install library
make install
```
3. Ensure dependencies are installed in your system:

3. Ensure dependencies are installed in your system:
   - Qt 6.4.0 and above: include Qt5compat module if not included by default
   - C++ compiler & CMake. Specific compiler depends on OS (gcc for ubuntu, clang for macos, msvc for windows etc.). For example `sudo apt install cmake build-essential libgl1-mesa-dev`
   - Python 3.10
   - zlib (This projects uses QuaZip for handling zip operations)
   - OpenGL dev
   - python venv (for venv to have it's own pip, not necessary to build but for kedro to execute)
   - xcb libraries (Ubuntu)
   `sudo apt install libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-xinerama0 libxcb-xkb1 libxkbcommon-x11-0 libxcb-cursor0 libxcb-cursor-dev`
   - build tools for QT
   `sudo apt install cmake build-essential libgl1-mesa-dev`
   - zlib: `sudo apt install zlib1g-dev libbz2-dev`
   - format: `sudo apt-get install libfmt-dev`
   - vulkan: `sudo apt install vulkan-tools vulkan-validationlayers-dev libvulkan-dev`
   - Optional dependencies:
     - CMake gui
     - vscode
     - extensions if using vscode:
       - CMake Tools
       - C/C ++ Extension Pack
     - MSVC if on Windows
4. Launch the build

```bash
mkdir build && cd build
cmake ../app && cmake
```

- replace `%BUILD_DIR` with your target build dir
- replace `Debug` with `Release` for release build
- these commands are usually generated by either vscode extensions or cmake gui
- during the first build you will need to provide the path to your Qt `path/to/qt/version/os/lib/cmake/Qt6` for example: `/Users/%USER/Qt/6.7.1/macos/lib/cmake/Qt6`

### Windows Environment

0. cmake needs to be run from Windows, this means the following should be present in Windows:
 a. cmake
 b. Qt
 c. zlib
 d. bzip2
 e. miniconda, followed by a Python env

Most of the dependencies are to be installed similar to how it is done for Linux. Certain specific library like zlib needs to be compiled from source for Windows. The steps for the same are :

  1. In WSL, install mingw to compile for Windows from Linux (to compile zlib, easier inside wsl)
  2. zlib-1.3
    a. Clone the latest repo from "<https://github.com/madler/zlib>" to a Windows directory (D:/zlib-1.3/)
    b. Navigate to this directory in wsl and run the following commands
    c. CC=x86_64-w64-mingw32-gcc ./configure --prefix=/mnt/d/zlib-1.3/build --static
    d. make
    e. make install
    f. Check if lib, include are present in "d:/zlib-1.3/build"

Once all dependencies have been installed, in the "windows_toolchain.cmake" file inside "app" directory, set up all paths correctly. This should have windows paths to the compiler (qt_6.7.3/Tools/mingw1120_64/bin/gcc.exe etc.), Python, Qt6, zlib

Follow the remaining steps to build from PowerShell:

1. mkdir in app/ folder a new build_win
2. Set WIN_DEPLOY flag to ON in the CMakelists.txt
3. Run :

```bash
 cmake -S . -B build_win/ -DCMAKE_TOOLCHAIN_FILE="windows_toolchain.cmake" -G "MinGW Makefiles"
 cmake --build $BUILD_DIR --config Debug
```

- replace `%BUILD_DIR` with your target build dir
- replace `Debug` with `Release` for release build
