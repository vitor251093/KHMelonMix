cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_QT6=ON -B build || exit 1
cmake --build build -j$(nproc --all) || exit 1
./build/melonDS "rom/game.nds"