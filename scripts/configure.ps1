$ErrorActionPreference = "Stop"

cmake -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Debug
