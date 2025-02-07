#!/bin/bash

vulkan_dir="/Users/xiuhaozhong/DevEnvironments/VulkanSDK/1.4.304.0/macOS/"
shader_compiler="bin/glslc"

${vulkan_dir}${shader_compiler} shader.vert -o vert.spv
${vulkan_dir}${shader_compiler} shader.frag -o frag.spv
