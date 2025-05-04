# Cross-compiling for Windows on Linux Hosts

1. Build a cross-compiled version of GLEW:

```bash
sudo apt install python-is-python3

git clone https://github.com/nigels-com/glew.git
cd glew/auto
make glew.lib.static

# go back into the glew folder
cd ..

# build glew
make SYSTEM=linux-mingw64

# This is your PATH_TO_GLEW
pwd
```

2. Build a cross-compiled version of FreeGLUT:

```bash
git clone https://github.com/freeglut/freeglut.git
cd freeglut/

git checkout v3.6.0

GNU_HOST=x86_64-w64-mingw32
mkdir freeglut-x86_64-windows/ && cd freeglut-x86_64-windows/

cmake \
    -D GNU_HOST=$GNU_HOST \
    -D CMAKE_TOOLCHAIN_FILE=mingw_cross_toolchain.cmake \
    -D CMAKE_INSTALL_PREFIX=/freeglut \
    ..

make -j4
make install DESTDIR=$(pwd)

# This is your PATH_TO_FREEGLUT
echo $DESTDIR
```

3. Create the windows.config file with the following contents (update values where necessary):

```
# Change as appropriate...
PATH_TO_MINGW=/usr/lib/gcc/x86_64-w64-mingw32/13-win32/
PATH_TO_GLEW=/path/to/glew
PATH_TO_FREEGLUT=/path/to/freeglut/freeglut-x86_64-windows
```

The paths to GLEW and FreeGLUT should be 

4. Use the Windows spec and config for QMake

```bash
qmake6 -spec win32-g++ && make clean && make
```
