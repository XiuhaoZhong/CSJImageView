set shader_compiler=%VULKAN_SDK%\bin\glslc.exe

echo %VULKAN_SDK%

echo %shader_compiler%

start %shader_compiler% shader.vert -o vert.spv
start %shader_compiler% shader.frag -o frag.spv

pause