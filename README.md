# RHI
Render Hardware Interface. With DX11 and Vulkan backends

## Shader
All backends accept SPIR-V as the common shader format, and resource binding is done in the manner of Vulkan, i.e., using descriptor sets. As a result, SPIRV-Cross becomes a dependency for this project as non-vulkan backends need to translate SPIR-V binary into their respecting shader formats.

RHI just needs to link against `spirv-cross-glsl` target. Make sure this target is built or imported somewhere in your cmake project.
