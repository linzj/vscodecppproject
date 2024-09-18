@REM set BUILDTYPE=RelWithDebInfo
set BUILDTYPE=Debug

if not "%~1"=="" (
    set BUILDTYPE=%~1
)


cmake --preset=default -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=%BUILDTYPE% -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build
