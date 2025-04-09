# Replace all paths to the corresponding paths in your workspace
# Note the versions of all the libraries (qt, zlib, bzip2 etc)
# zlib may need to be built from the source after downloading

#Follow the remaining steps to build from PowerShell:

    # 1. mkdir in app/ folder a new build_win
    # 2. Run :
    # 	cmake -S . -B build_win/ -DCMAKE_TOOLCHAIN_FILE="windows_toolchain.cmake" -G "MinGW Makefiles"
    # 	cmake --build ./build_win
    # 3. Navigate to the build_win directory (where executable is located), open PowerShell and run
    # 	D:\qt_6.7.3\6.7.3\mingw_64\bin\windeployqt.exe .\DescartesBuilder.exe # copies the required dlls
    # 	cp D:\qt_6.7.3\6.7.3\mingw_64\bin\Qt6Core5Compat.dll . # copy one additional required dll
    # 4. You should be able to run the .exe from the terminal or the folder now.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)


set(CMAKE_C_COMPILER "D:/qt_6.7.3/Tools/mingw1120_64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "D:/qt_6.7.3/Tools/mingw1120_64/bin/g++.exe")
set(CMAKE_RC_COMPILER "D:/qt_6.7.3/Tools/mingw1120_64/bin/windres")

set(CMAKE_FIND_ROOT_PATH "D:/qt_6.7.3/Tools/mingw1120_64")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Convert paths to Windows style using wslpath
set(Python3_ROOT_DIR "D:/miniconda_windows/envs/builder_env_win")
set(PYTHON3_INCLUDE_DIR "D:/miniconda_windows/envs/builder_env_win/include")
set(PYTHON3_LIBRARY "D:/miniconda_windows/envs/builder_env_win/libs/python310.lib")
set(Python3_EXECUTABLE "D:/miniconda_windows/envs/builder_env_win/python.exe")

set(ZLIB_LIBRARY "D:/zlib-1.3/build/lib/libz.a" CACHE FILEPATH "Path to Zlib library")
set(ZLIB_INCLUDE_DIR "D:/zlib-1.3/build/include" CACHE FILEPATH "Path to Zlib include directory")
set(CMAKE_PREFIX_PATH "D:/zlib-1.3/build")

set(BZIP2_LIBRARIES "D:/bzip2_1.0.8/libbz2.lib")
set(BZIP2_INCLUDE_DIR "D:/bzip2_1.0.8")
set(CMAKE_PREFIX_PATH "D:/bzip2_1.0.8")

# Set the root directory of your Qt installation
set(Qt6_DIR "D:/qt_6.7.3/6.7.3/mingw_64/lib/cmake/Qt6")
# Ensure CMake knows where to find Qt
set(CMAKE_PREFIX_PATH "D:/qt_6.7.3/6.7.3/mingw_64")