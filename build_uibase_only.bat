@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=amd64
if errorlevel 1 exit /b %errorlevel%
cd /d D:\Projects\modorganizer-uibase-game-archive-handler
cmake -S . -B build-local -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/EHsc /MP /W4" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -DVCPKG_MANIFEST_NO_DEFAULT_FEATURES=ON -DVCPKG_MANIFEST_FEATURES=testing -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
if errorlevel 1 exit /b %errorlevel%
cmake --build build-local -j 8
if errorlevel 1 exit /b %errorlevel%
cmake --install build-local
if errorlevel 1 exit /b %errorlevel%
echo UIBASE_ONLY_BUILD_DONE
exit /b 0
