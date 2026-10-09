## Sobre
Shaderc é o utilitário para build dos shaders para o bgfx.

## Build do executável

:: 1. Clone o repositório bgfx.cmake

`git clone https://github.com/bkaradzic/bgfx.cmake.git`

`cd bgfx.cmake`


:: 2. Inicialize e atualize os submódulos (bgfx, bx, bimg)

`git submodule update --init --recursive`


:: 3. Crie e entre no diretório de build

`mkdir build`

`cd build`


:: 4. Configure com CMake (use o gerador de sua preferência)

- Para Ninja (recomendado para velocidade): `cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..`

- Para Visual Studio 2022: `cmake -G"Visual Studio 17 2022" -A x64 ..`


:: 5. Compile
`cmake --build . --config Release`

## Exemplo de uso
`.\shaderc.exe -f "shaders/sprite.vs.sc" -o "shaders/sprite.vs.bin" --platform windows --profile s_5_0 --type vertex --varyingdef "shaders/varying.def.sc" --verbose`

`.\shaderc.exe -f "shaders/sprite.fs.sc" -o "shaders/sprite.fs.bin" --platform windows --profile s_5_0 --type fragment --varyingdef "shaders/varying.def.sc" --verbose`

caso linux: 
--platform linux
--profile spirv (vulkan)
--profile 150 (opengl) ... 120, 130, 140, 150, 330, 400, 430

## Sobre
https://bkaradzic.github.io/bgfx/tools.html#shader-compiler-shaderc