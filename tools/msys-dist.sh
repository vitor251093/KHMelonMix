#!/bin/bash

if [[ ! -x MelonMix.exe ]]; then
	echo "Run this script from the directory you built MelonMix."
	exit 1
fi

mkdir -p dist

for lib in $(ldd MelonMix.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
	cp "${lib}" dist
done

cp MelonMix.exe dist
windeployqt dist
