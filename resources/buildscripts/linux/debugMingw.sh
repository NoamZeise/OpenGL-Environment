cd build
cmake .. -G"Ninja Multi-Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
cmake --build . --config Debug
cd ..
cp -r resources/audio build/Debug
cp -r resources/shaders build/Debug
cp -r resources/models build/Debug
cp -r resources/textures build/Debug
cp -r resources/mingw-dlls/. build/Debug
cd build/Debug
wine OpenGL-Environment.exe
