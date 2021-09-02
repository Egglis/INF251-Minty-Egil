rem @echo off

md ext
cd ext

rd /S /Q glfw
git clone https://github.com/glfw/glfw.git

rd /S /Q glm
git clone https://github.com/g-truc/glm.git
cd glm
git reset --hard 2bd42176855ac34c8f362df5a5cf235db36d27f1
cd ..

rd /S /Q glbinding
git clone https://github.com/cginternals/glbinding.git
cd glbinding
git reset --hard 28d32d9bbc72aedf815f18113b0bd3aa7b354108
cd ..

rd /S /Q globjects
git clone https://github.com/cginternals/globjects.git
cd globjects
git reset --hard dc68b09a53ec20683d3b3a12ed8d9cb12602bb9a
cd ..

cd ..
md lib
cd lib

rd /S /Q imgui
git clone https://github.com/ocornut/imgui.git

rd /S /Q tinyfd
git clone http://git.code.sf.net/p/tinyfiledialogs/code tinyfd

rd /S /Q stb
git clone https://github.com/nothings/stb.git

cd ..