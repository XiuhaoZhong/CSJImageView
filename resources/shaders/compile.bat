set shader_compiler=%VULKAN_SDK%\bin\glslc.exe

echo %VULKAN_SDK%

echo %shader_compiler%

start %shader_compiler% shader.vert -o vert.spv
start %shader_compiler% shader.frag -o frag.spv
start %shader_compiler% yuv_generator.comp -o yuv_generator.spv
start %shader_compiler% yuv_shader.vert -o yuv_vert.spv
start %shader_compiler% yuv_shader.frag -o yuv_frag.spv

pause