rm -rf ./build
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
cd build