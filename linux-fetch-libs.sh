#!/usr/bin/bash

echo "Downloading external dependencies..."

[ -d ./ext ] && rm -rf ./ext

mkdir ext
cd ext

git clone https://github.com/glfw/glfw.git

git clone https://github.com/g-truc/glm.git
cd glm
git reset --hard 2bd42176855ac34c8f362df5a5cf235db36d27f1
cd ..

git clone https://github.com/cginternals/glbinding.git

cd glbinding
git reset --hard 28d32d9bbc72aedf815f18113b0bd3aa7b354108
cd ..

git clone https://github.com/cginternals/globjects.git
cd globjects
git reset --hard dc68b09a53ec20683d3b3a12ed8d9cb12602bb9a
cd ..

cd ..

[ ! -d ./lib ] && mkdir lib
cd lib

[ -d ./imgui ] && rm -rf imgui
git clone https://github.com/ocornut/imgui.git

[ -d ./tinyfd ] && rm -rf tinyfd
git clone http://git.code.sf.net/p/tinyfiledialogs/code tinyfd

[ -d ./stb ] && rm -rf stb
git clone https://github.com/nothings/stb.git

cd ..

echo "Done."
