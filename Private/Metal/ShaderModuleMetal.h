#pragma once
#include "MtlCommon.h"
#include "SPIRVToMSL.h"
#include "ShaderModule.h"

namespace RHI
{

class CShaderModuleMetal : public CShaderModule
{
public:
    typedef std::shared_ptr<CShaderModuleMetal> Ref;

    CShaderModuleMetal(CDeviceMetal& parent, size_t size, const void* pCode);
    ~CShaderModuleMetal() override;

    const std::vector<CPipelineResource>& GetShaderResources() const override
    {
        return ShaderResources;
    }

    id GetMTLFunction() const { return Function; }
    id GetMTLLibrary() const { return Library; }
    const CMSLBindingRemap& GetBindingRemap() const { return BindingRemap; }

private:
    id Library;
    id Function;
    std::vector<CPipelineResource> ShaderResources;
    CMSLBindingRemap BindingRemap;
};

} /* namespace RHI */
