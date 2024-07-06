cmake -DCMAKE_BUILD_TYPE=Debug -B build || exit 1
cmake --build build -j$(nproc --all) || exit 1
./build/melonDS