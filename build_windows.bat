mkdir _debugbuild
cd _debugbuild
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_CUDA=OFF -DENABLE_WINDOWS=ON -DENABLE_LINUX=OFF -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -A x64 ../base 
cmake --build .
pause