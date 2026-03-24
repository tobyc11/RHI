#include "SamplerMetal.h"
#include "DeviceMetal.h"
#include "MtlHelpers.h"
#include "RHIException.h"

namespace RHI
{

CSamplerMetal::CSamplerMetal(CDeviceMetal& parent, const CSamplerDesc& desc)
{
    id<MTLDevice> device = (id<MTLDevice>)parent.GetMTLDevice();

    MTLSamplerDescriptor* sd = [[MTLSamplerDescriptor alloc] init];
    sd.magFilter = MtlCast(desc.MagFilter);
    sd.minFilter = MtlCast(desc.MinFilter);
    sd.mipFilter = MtlCast(desc.MipmapMode);
    sd.sAddressMode = MtlCast(desc.AddressModeU);
    sd.tAddressMode = MtlCast(desc.AddressModeV);
    sd.rAddressMode = MtlCast(desc.AddressModeW);
    sd.lodMinClamp = desc.MinLod < 0.0f ? 0.0f : desc.MinLod;
    sd.lodMaxClamp = desc.MaxLod;
    sd.maxAnisotropy = desc.AnisotropyEnable ? static_cast<NSUInteger>(desc.MaxAnisotropy) : 1;
    sd.compareFunction =
        desc.CompareEnable ? MtlCast(desc.CompareOp) : MTLCompareFunctionNever;
    sd.supportArgumentBuffers = YES;

    Sampler = [device newSamplerStateWithDescriptor:sd];
    if (!Sampler)
        throw CRHIRuntimeError("Failed to create Metal sampler");
}

CSamplerMetal::~CSamplerMetal() { }

} /* namespace RHI */
