# Setup

This document explains how to install the dependencies and build **Ghost in the Maze** on Linux.

## 1. Project Requirements

The project is built with:

- **C++**
- **Qt** with OpenGL support
- **GLM** for math utilities
- **Assimp** for loading 3D models

The repository already includes the required source files for GLM and the Assimp helper classes. On most systems, you only need the Qt development tools and the Assimp static libraries.

## 2. Install System Dependencies

On Ubuntu or Debian-based systems, install the Qt development tools and the packages commonly needed to build the project:

```bash
sudo apt update
sudo apt install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools build-essential
```

If your distribution separates OpenGL support packages, install them as well:

```bash
sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev
```

## 3. Assimp Libraries

The `.pro` file links against the following Assimp static libraries:

- `libassimp.a`
- `libdraco.a`
- `libkubazip.a`
- `libminizip.a`
- `libpoly2tri.a`
- `libpolyclipping.a`
- `libpugixml.a`
- `libz.a`

If these libraries are already present in `assimp/lib`, you can skip this step.

If they are missing, the recommended approach is to install Assimp with **vcpkg** and copy the generated static libraries into the project folder:

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install assimp
```

After installing Assimp, copy the `.a` files from the vcpkg installation directory into:

```bash
<project-folder>/assimp/lib/
```

## 4. Build the Project

The project uses a Qt `.pro` file, so the simplest way to build it is with `qmake` and `make`:

```bash
cd <project-folder>
mkdir -p build && cd build
qmake ../BaseGLWidget.pro
make -j$(nproc)
```

Replace `<project-folder>` with the directory name you used when cloning or copying the repository.

If your system uses `qmake6` instead of `qmake`, replace the `qmake` command accordingly.

## 5. Run the Application

After a successful build, run the generated binary from the build directory:

```bash
./BaseGLWidget
```

Depending on your Qt version and build environment, the executable name may vary slightly. If that happens, check the contents of the `build/` directory and run the generated application binary.

## 6. Open the Project in Qt Creator

You can also open `BaseGLWidget.pro` directly in **Qt Creator**:

1. Open Qt Creator.
2. Choose **Open Project**.
3. Select `BaseGLWidget.pro`.
4. Configure the kit and build directory.
5. Build and run the project from the IDE.

## 7. Repository Structure

- `src/` - main application sources
- `ui/` - Qt Designer UI files
- `models/` - 3D models and material files
- `shaders/` - OpenGL shaders
- `images/` - screenshots used in the README
- `assimp/` - Assimp helper code and local dependencies
- `glm-master/` - GLM headers