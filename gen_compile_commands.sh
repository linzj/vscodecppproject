BUILDTYPE=Debug #RelWithDebInfo
if [ $# -ne 0 ]; then
  BUILDTYPE=$1
fi

cmake --preset=default -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$BUILDTYPE
cmake --build build
