set(MODULE_NAME RHI)

set(CMAKE_CXX_STANDARD 17)

file(GLOB RHI_PRIVATE_SOURCES Private/*.h Private/*.cpp)
file(GLOB RHI_PRIVATE_DIRECT3D11_SOURCES Private/Direct3D11/*.h Private/Direct3D11/*.cpp)
file(GLOB RHI_PUBLIC_SOURCES Public/*.h)
source_group(Private FILES ${RHI_PRIVATE_SOURCES})
source_group(Private\\Direct3D11 FILES ${RHI_PRIVATE_DIRECT3D11_SOURCES})
source_group(Public FILES ${RHI_PUBLIC_SOURCES})

add_library(${MODULE_NAME} ${RHI_PRIVATE_SOURCES} ${RHI_PRIVATE_DIRECT3D11_SOURCES} ${RHI_PUBLIC_SOURCES})

include(GenerateExportHeader)
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
generate_export_header(${MODULE_NAME} EXPORT_MACRO_NAME RHI_API EXPORT_FILE_NAME RHIModuleAPI.h)

target_include_directories(${MODULE_NAME} PRIVATE Private)
target_include_directories(${MODULE_NAME} PUBLIC Public)
target_compile_options(${MODULE_NAME} ${DEFAULT_COMPILE_OPTIONS})

target_link_libraries(${MODULE_NAME} PRIVATE Foundation)
target_link_libraries(${MODULE_NAME} PRIVATE Math)

target_compile_definitions(${MODULE_NAME} PUBLIC RHI_IMPL_DIRECT3D11)

target_link_libraries(${MODULE_NAME} PRIVATE d3d11.lib d3dcompiler.lib dxgi.lib dxguid.lib)
