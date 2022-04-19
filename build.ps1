New-Item -Type Directory -Force -Path vsbuild | Out-Null

Push-Location vsbuild

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_MESSAGE=NEVER `
    --log-level=STATUS --no-warn-unused-cli -G "Visual Studio 16 2019" -A x64 `
    -DCMAKE_INSTALL_PREFIX:PATH="I:\Projects\ModOrganizer2\mo2-mdc\install" `
    -DDEPENDENCIES_DIR="I:\Projects\ModOrganizer2\mo2-mdc\build" `
    -DBOOST_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\boost_1_75_0" `
    -DBOOST_LIBRARYDIR="I:\Projects\ModOrganizer2\mo2-mdc\build\boost_1_75_0\lib64-msvc-14.2\lib" `
    -DFMT_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\fmt-7.1.3" `
    -DSPDLOG_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\spdlog-v1.8.2" `
    -DLOOT_PATH="I:\Projects\ModOrganizer2\mo2-mdc\build\libloot-0.16.3-0-g2ffdac5_0.16.3-win64" `
    -DLZ4_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\lz4-v1.9.3" `
    -DQT_ROOT="I:\Languages\Qt\5.15.2\msvc2019_64" `
    -DZLIB_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\zlib-1.2.11" `
    -DPYTHON_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\python-3.8.7" `
    -DSEVENZ_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\7zip-19.00" `
    -DLIBBSARCH_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\libbsarch-0.0.8-release-x64" `
    -DBOOST_DI_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\di" `
    -DGTEST_ROOT="I:\Projects\ModOrganizer2\mo2-mdc\build\googletest" ..

Pop-Location
