#pragma once

#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

#include <stdexcept>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>

// When included from plain C++ (.cpp), provide opaque typedefs so headers compile.
// Actual Objective-C types are only available in .mm translation units.
#ifndef __OBJC__
using id = void*;
#define nil nullptr

// Forward-declare protocol-typed pointers as void* for C++ headers
#define MTL_FWD_DECL(name) using name = void*
MTL_FWD_DECL(MTLDevice_id);
MTL_FWD_DECL(MTLCommandQueue_id);
MTL_FWD_DECL(MTLBuffer_id);
MTL_FWD_DECL(MTLTexture_id);
MTL_FWD_DECL(MTLSamplerState_id);
MTL_FWD_DECL(MTLLibrary_id);
MTL_FWD_DECL(MTLFunction_id);
MTL_FWD_DECL(MTLRenderPipelineState_id);
MTL_FWD_DECL(MTLComputePipelineState_id);
MTL_FWD_DECL(MTLDepthStencilState_id);
MTL_FWD_DECL(MTLRenderCommandEncoder_id);
MTL_FWD_DECL(MTLComputeCommandEncoder_id);
MTL_FWD_DECL(MTLBlitCommandEncoder_id);
MTL_FWD_DECL(MTLCommandBuffer_id);
MTL_FWD_DECL(MTLParallelRenderCommandEncoder_id);
MTL_FWD_DECL(CAMetalDrawable_id);
#undef MTL_FWD_DECL

class CAMetalLayer;
class MTLRenderPassDescriptor;
using NSUInteger = unsigned long;
using NSError = void;

enum MTLCullMode : NSUInteger { MTLCullModeNone = 0, MTLCullModeFront = 1, MTLCullModeBack = 2 };
enum MTLTriangleFillMode : NSUInteger { MTLTriangleFillModeFill = 0, MTLTriangleFillModeLines = 1 };
enum MTLWinding : NSUInteger { MTLWindingClockwise = 0, MTLWindingCounterClockwise = 1 };
enum MTLPrimitiveType : NSUInteger {};
enum MTLIndexType : NSUInteger { MTLIndexTypeUInt16 = 0, MTLIndexTypeUInt32 = 1 };
#endif

namespace RHI
{

class CDeviceMetal;

#ifdef __OBJC__
inline void MtlCheckError(NSError* error, const char* file, int line)
{
    if (error)
    {
        std::string msg = std::string("Metal error at ") + file + ":" + std::to_string(line) + " - "
            + [[error localizedDescription] UTF8String];
        throw std::runtime_error(msg);
    }
}

#define MTL_CHECK(error) ::RHI::MtlCheckError(error, __FILE__, __LINE__)
#endif

} /* namespace RHI */
