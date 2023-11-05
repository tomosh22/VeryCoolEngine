%VULKAN_SDK%\Bin\glslc.exe block.vert -o blockVert.spv
%VULKAN_SDK%\Bin\glslc.exe block.frag -o blockFrag.spv
%VULKAN_SDK%\Bin\glslc.exe fullscreen.vert -o fullscreenVert.spv
%VULKAN_SDK%\Bin\glslc.exe fullscreen.frag -o fullscreenFrag.spv
%VULKAN_SDK%\Bin\glslc.exe mesh.vert -o meshVert.spv
%VULKAN_SDK%\Bin\glslc.exe mesh.frag -o meshFrag.spv
%VULKAN_SDK%\Bin\glslc.exe meshGBuffer.frag -o meshGBufferFrag.spv
%VULKAN_SDK%\Bin\glslc.exe mesh.tesc -o meshTesc.spv
%VULKAN_SDK%\Bin\glslc.exe mesh.tese -o meshTese.spv


%VULKAN_SDK%\Bin\glslc.exe copyToFrameBuffer.vert -o copyToFrameBufferVert.spv
%VULKAN_SDK%\Bin\glslc.exe copyToFrameBuffer.frag -o copyToFrameBufferFrag.spv
pause