#pragma once
#include "Format.h"
#include "RHICommon.h"
#include "Resources.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RHI
{

class CRenderGraph;

enum ERenderNodeType : uint32_t
{
    RenderPass,
    RenderResource
};

class CRenderNode
{
public:
    CRenderNode(CRenderGraph& g, std::string name, ERenderNodeType t)
        : Graph(g)
        , Name(std::move(name))
        , Type(t)
    {
    }

    CRenderGraph& GetGraph() const { return Graph; }
    const std::string& GetName() const { return Name; }
    ERenderNodeType GetType() const { return Type; }

    // Temporary, for traversal use
    uint32_t _Visited;
    size_t _PassOrder;

private:
    CRenderGraph& Graph;
    std::string Name;
    ERenderNodeType Type;
};

// Named this way because of the low level construct CRenderPass
class CGraphRenderPass : public CRenderNode
{
public:
    CGraphRenderPass(CRenderGraph& g, std::string name)
        : CRenderNode(g, std::move(name), ERenderNodeType::RenderPass)
    {
    }

    // Could be read-write dependency
    void AddColorAttachment(const std::string& resource, uint32_t index, bool read = true,
                            bool write = true);
    // Could be read-write dependency
    void AddDepthStencilAttachment(const std::string& resource, bool read = true,
                                   bool write = true);
    // A read-only dependency. Sampled image in a shader (fragment shader assumed)
    void AddShaderResource(const std::string& resource);
};

class CRenderResource : public CRenderNode
{
public:
    CRenderResource(CRenderGraph& g, std::string name, EFormat format)
        : CRenderNode(g, std::move(name), ERenderNodeType::RenderResource)
        , Format(format)
    {
    }

    EFormat GetFormat() const { return Format; }

    CImageView::Ref GetImageView() const;

private:
    EFormat Format;
};

enum EResourceUsageType : uint32_t
{
    ColorAttachment,
    DepthStencilAttachment,
    ShaderResource
};

// This class represents an edge
struct CResourceUsage
{
    bool bRead : 1;
    bool bWrite : 1;
    EResourceUsageType Type;
    uint32_t ColorAttachmentIndex;
    EResourceState RequiredState;
};

class CRenderGraph
{
    friend class CGraphRenderPass;

public:
    struct CTransition
    {
        size_t NodeId;
        EResourceState StateDuring;
        EResourceState StateAfter;

        bool IsUnneeded() const;
    };

    CRenderGraph();

    CRenderResource& AddTransientResource(const std::string& name, EFormat format);
    CGraphRenderPass& AddRenderPass(const std::string& name);
    void RemoveRenderPass(const std::string& name);
    void SetGoal(const std::string& name);

    bool Validate() const;
    void Bake() const;

private:
    void ValidateDFSRenderPass(size_t nodeId) const;
    void ValidateDFSResource(size_t nodeId) const;
    std::shared_ptr<CResourceUsage> AddEdge(size_t src, size_t dst);

    std::list<size_t> FreeNodeIds;

    std::vector<std::shared_ptr<CRenderNode>> Nodes;
    std::vector<std::map<size_t, std::shared_ptr<CResourceUsage>>> AdjLists;
    std::unordered_map<std::string, size_t> NameToNodeId;

    size_t GoalNode;
    mutable bool ValidateSuccess;
    mutable uint32_t DFSDepth;
    mutable std::vector<size_t> PassOrder; // The pass at each time step
    mutable std::vector<std::vector<CTransition>> Transitions; // Transitions at each time step
};

} /* namespace RHI */
