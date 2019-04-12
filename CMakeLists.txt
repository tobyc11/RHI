option(RHI_BACKEND_DIRECT3D11 "Use Direct3D 11 as the backend" OFF)
option(RHI_BACKEND_VULKAN "Use Vulkan as the backend" ON)

set(MODULE_NAME RHI)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET spirv-cross-glsl)
    message(WARNING "RHI cannot find target spirv-cross-glsl. Build will likely fail. Is SPIRV-Cross included with your project?")
endif()

if(RHI_BACKEND_DIRECT3D11)
    file(GLOB RHI_PRIVATE_DIRECT3D11_SOURCES Private/Direct3D11/*.h Private/Direct3D11/*.cpp)
    source_group(Private\\Direct3D11 FILES ${RHI_PRIVATE_DIRECT3D11_SOURCES})
    add_library(Backend INTERFACE)
    target_compile_definitions(Backend INTERFACE RHI_IMPL_DIRECT3D11)
    add_library(BackendPriv INTERFACE)
    target_link_libraries(BackendPriv INTERFACE d3d11.lib d3dcompiler.lib dxgi.lib dxguid.lib)
    target_link_libraries(BackendPriv INTERFACE spirv-cross-hlsl) #spirv must be included in the project
elseif(RHI_BACKEND_VULKAN)
    find_package(Vulkan REQUIRED)

    file(GLOB RHI_PRIVATE_VULKAN_SOURCES Private/Vulkan/*.h Private/Vulkan/*.cpp)
    source_group(Private\\Vulkan FILES ${RHI_PRIVATE_VULKAN_SOURCES})
    add_library(Backend INTERFACE)
    target_compile_definitions(Backend INTERFACE RHI_IMPL_VULKAN)
    add_library(BackendPriv INTERFACE)
    target_link_libraries(BackendPriv INTERFACE Vulkan::Vulkan)
    target_link_libraries(BackendPriv INTERFACE spirv-cross-glsl) #spirv must be included in the project
endif()

file(GLOB RHI_PRIVATE_SOURCES Private/*.h Private/*.cpp)
file(GLOB RHI_PUBLIC_SOURCES Public/*.h)
source_group(Private FILES ${RHI_PRIVATE_SOURCES})
source_group(Public FILES ${RHI_PUBLIC_SOURCES})

#Add the library and link against Backend
add_library(${MODULE_NAME} ${RHI_PRIVATE_SOURCES} ${RHI_PRIVATE_DIRECT3D11_SOURCES} ${RHI_PRIVATE_VULKAN_SOURCES} ${RHI_PUBLIC_SOURCES})
add_library(tc::${MODULE_NAME} ALIAS ${MODULE_NAME})
target_link_libraries(${MODULE_NAME} PUBLIC Backend)
target_link_libraries(${MODULE_NAME} PUBLIC BackendPriv)

target_include_directories(${MODULE_NAME} PRIVATE Private)
target_include_directories(${MODULE_NAME} PUBLIC Public)
target_compile_options(${MODULE_NAME} ${DEFAULT_COMPILE_OPTIONS})

#Generate dll linkage header
include(GenerateExportHeader)
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
generate_export_header(${MODULE_NAME} EXPORT_MACRO_NAME RHI_API EXPORT_FILE_NAME RHIModuleAPI.h)

#Dependencies on other modules
target_link_libraries(${MODULE_NAME} PUBLIC Foundation)
if (TARGET imgui)
	target_link_libraries(${MODULE_NAME} PUBLIC imgui)
	target_compile_definitions(${MODULE_NAME} PRIVATE RHI_HAS_IMGUI)
endif()
