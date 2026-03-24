#include "ShaderModuleMetal.h"
#include "DeviceMetal.h"

namespace RHI
{

CShaderModuleMetal::CShaderModuleMetal(CDeviceMetal& parent, size_t size, const void* pCode)
    : CShaderModule(pCode, size, DXBCBlob {})
{
    const uint32_t* spirvWords = static_cast<const uint32_t*>(pCode);
    size_t wordCount = size / sizeof(uint32_t);

    CMSLCompileResult compiled = CompileSPIRVToMSL(spirvWords, wordCount);

    ShaderResources = std::move(compiled.Resources);
    BindingRemap = std::move(compiled.Remap);

    NSString* source = [NSString stringWithUTF8String:compiled.Source.c_str()];
    NSError* error = nil;
    Library = [parent.GetMTLDevice() newLibraryWithSource:source options:nil error:&error];
    MTL_CHECK(error);

    Function = [Library newFunctionWithName:@"main0"];
    if (!Function)
    {
        // Some SPIRV-Cross versions may use a different name
        Function = [Library newFunctionWithName:@"main"];
        if (!Function)
            throw CRHIRuntimeError("Failed to find entry point in compiled MSL shader");
    }
}

CShaderModuleMetal::~CShaderModuleMetal() { }

} /* namespace RHI */
