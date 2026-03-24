#pragma once
#include "MtlCommon.h"
#include "Resources.h"

namespace RHI
{

class CImageMetal : public CImage
{
public:
    typedef std::shared_ptr<CImageMetal> Ref;

    ~CImageMetal() override = default;

    EFormat GetFormat() const override { return Format; }
    EImageUsageFlags GetUsageFlags() const override { return Usage; }
    uint32_t GetWidth() const override { return Width; }
    uint32_t GetHeight() const override { return Height; }
    uint32_t GetDepth() const override { return Depth; }
    uint32_t GetMipLevels() const override { return MipLevels; }
    uint32_t GetArrayLayers() const override { return ArrayLayers; }
    uint32_t GetSampleCount() const override { return SampleCount; }

    virtual id GetMTLTexture() const = 0;

protected:
    EFormat Format;
    EImageUsageFlags Usage;
    uint32_t Width = 1;
    uint32_t Height = 1;
    uint32_t Depth = 1;
    uint32_t MipLevels = 1;
    uint32_t ArrayLayers = 1;
    uint32_t SampleCount = 1;
};

class CMemoryImageMetal : public CImageMetal
{
public:
    CMemoryImageMetal(CDeviceMetal& parent, unsigned long type, EFormat format,
                      EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t depth,
                      uint32_t mipLevels, uint32_t arrayLayers, uint32_t sampleCount,
                      const void* initialData);
    ~CMemoryImageMetal() override;

    id GetMTLTexture() const override { return Texture; }

private:
    CDeviceMetal& Parent;
    id Texture;
};

class CSwapChainImageMetal : public CImageMetal
{
public:
    CSwapChainImageMetal(EFormat format, uint32_t width, uint32_t height);

    id GetMTLTexture() const override { return Texture; }
    void SetTexture(id tex);
    void SetSize(uint32_t w, uint32_t h);

private:
    id Texture;
};

} /* namespace RHI */
