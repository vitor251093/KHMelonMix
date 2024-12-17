cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_QT6=ON -B build || exit 1
cmake --build build -j$(nproc --all) || exit 1

# ./build/melonDS

./build/melonDS "roms/days.nds"
# ./build/melonDS "roms/days_eu.nds"
# ./build/melonDS "roms/days_jp.nds"
# ./build/melonDS "roms/days_jp_rev1.nds"

# ./build/melonDS "roms/recoded_us.nds"
# ./build/melonDS "roms/recoded_eu.nds"
# ./build/melonDS "roms/recoded_jp.nds"
