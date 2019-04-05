#include "RenderGraph.h"

namespace RHI
{

void CGraphRenderPass::AddColorAttachment(const std::string& resource, uint32_t index, bool read,
                                          bool write)
{
    assert(GetGraph().NameToNodeId.find(resource) != GetGraph().NameToNodeId.end());
    size_t src = GetGraph().NameToNodeId[GetName()];
    size_t dst = GetGraph().NameToNodeId[resource];
    auto usage = GetGraph().AddEdge(src, dst);
    usage->Type = EResourceUsageType::ColorAttachment;
    usage->bRead = read;
    usage->bWrite = write;
    usage->ColorAttachmentIndex = index;
    usage->RequiredState = EResourceState::RenderTarget;
}

void CGraphRenderPass::AddDepthStencilAttachment(const std::string& resource, bool read, bool write)
{
    assert(GetGraph().NameToNodeId.find(resource) != GetGraph().NameToNodeId.end());
    size_t src = GetGraph().NameToNodeId[GetName()];
    size_t dst = GetGraph().NameToNodeId[resource];
    auto usage = GetGraph().AddEdge(src, dst);
    usage->Type = EResourceUsageType::DepthStencilAttachment;
    usage->bRead = read;
    usage->bWrite = write;
    usage->RequiredState = EResourceState::DepthStencil;
}

void CGraphRenderPass::AddShaderResource(const std::string& resource)
{
    assert(GetGraph().NameToNodeId.find(resource) != GetGraph().NameToNodeId.end());
    size_t src = GetGraph().NameToNodeId[GetName()];
    size_t dst = GetGraph().NameToNodeId[resource];
    auto usage = GetGraph().AddEdge(src, dst);
    usage->Type = EResourceUsageType::ShaderResource;
    usage->bRead = true;
    usage->bWrite = false;
    usage->RequiredState = EResourceState::ShaderResource;
}

CRenderGraph::CRenderGraph()
{
    GoalNode = SIZE_MAX;
    Nodes.reserve(128);
    AdjLists.reserve(128);
}

CRenderResource& CRenderGraph::AddTransientResource(const std::string& name, EFormat format)
{
    assert(NameToNodeId.find(name) == NameToNodeId.end());
    auto node = std::make_shared<CRenderResource>(*this, name, format);
    size_t nextId;
    if (!FreeNodeIds.empty())
    {
        nextId = FreeNodeIds.front();
        FreeNodeIds.pop_front();
        Nodes[nextId] = node;
    }
    else
    {
        nextId = Nodes.size();
        Nodes.push_back(node);
        AdjLists.resize(Nodes.size());
    }
    NameToNodeId[name] = nextId;
    return *node;
}

CGraphRenderPass& CRenderGraph::AddRenderPass(const std::string& name)
{
    assert(NameToNodeId.find(name) == NameToNodeId.end());
    auto node = std::make_shared<CGraphRenderPass>(*this, name);
    size_t nextId;
    if (!FreeNodeIds.empty())
    {
        nextId = FreeNodeIds.front();
        FreeNodeIds.pop_front();
        Nodes[nextId] = node;
    }
    else
    {
        nextId = Nodes.size();
        Nodes.push_back(node);
        AdjLists.resize(Nodes.size());
    }
    NameToNodeId[name] = nextId;
    return *node;
}

void CRenderGraph::RemoveRenderPass(const std::string& name)
{
    assert(NameToNodeId.find(name) != NameToNodeId.end());
    auto id = NameToNodeId[name];
    assert(Nodes[id]->GetType() == ERenderNodeType::RenderPass);
    Nodes[id].reset();
    AdjLists[id].clear();
    NameToNodeId.erase(name);
    FreeNodeIds.push_back(id);
}

void CRenderGraph::SetGoal(const std::string& name)
{
    if (name.empty())
    {
        GoalNode = SIZE_MAX;
        return;
    }
    assert(NameToNodeId.find(name) != NameToNodeId.end());
    GoalNode = NameToNodeId[name];
}

void CRenderGraph::ValidateDFSRenderPass(size_t nodeId) const
{
    assert(nodeId >= 0);
    assert(nodeId < Nodes.size());
    assert(Nodes[nodeId]);
    auto node = Nodes[nodeId];
    assert(node->GetType() == ERenderNodeType::RenderPass);
    if (node->_Visited == 1)
    {
        // Back-edge
        ValidateSuccess = false;
        return;
    }
    if (node->_Visited == 2)
        return; // Cross edge
    DFSDepth++;
    node->_Visited = 1;
    std::cout << std::string(DFSDepth, ' ') << "[" << node->GetName() << "]" << std::endl;
    PassOrder.push_back(nodeId);
    for (const auto& pair : AdjLists[nodeId])
    {
        // If read-only, must be an input or srv
        if (pair.second->bRead && !pair.second->bWrite)
        {
            ValidateDFSResource(pair.first);
        }
    }
    node->_Visited = 2;
    DFSDepth--;
}

void CRenderGraph::ValidateDFSResource(size_t nodeId) const
{
    assert(nodeId >= 0);
    assert(nodeId < Nodes.size());
    assert(Nodes[nodeId]);
    auto node = Nodes[nodeId];
    assert(node->GetType() == ERenderNodeType::RenderResource);
    if (node->_Visited == 1)
    {
        // Back-edge
        ValidateSuccess = false;
        return;
    }
    if (node->_Visited == 2)
        return; // Cross edge
    DFSDepth++;
    node->_Visited = 1;
    std::cout << std::string(DFSDepth, ' ') << node->GetName() << std::endl;
    unsigned writerCount = 0;
    for (const auto& pair : AdjLists[nodeId])
    {
        // Anything that writes me is noteworthy
        if (pair.second->bWrite)
        {
            writerCount++;
            if (writerCount > 1)
            {
                ValidateSuccess = false;
                std::cout << "Currently does not support a resource having multiple writers."
                          << std::endl;
            }
            ValidateDFSRenderPass(pair.first);
        }
    }
    node->_Visited = 2;
    DFSDepth--;
}

bool CRenderGraph::Validate() const
{
    if (GoalNode == SIZE_MAX)
        return false;
    ValidateSuccess = true;

    for (auto node : Nodes)
        if (node)
            node->_Visited = 0;

    DFSDepth = 0;
    PassOrder.clear();
    ValidateDFSResource(GoalNode);

    return ValidateSuccess;
}

void CRenderGraph::Bake() const
{
    if (!ValidateSuccess)
        ;

    std::reverse(PassOrder.begin(), PassOrder.end());
    size_t index = 0;
    for (size_t nodeId : PassOrder)
        Nodes[nodeId]->_PassOrder = index++;

    Transitions.clear();
    Transitions.resize(PassOrder.size());

    for (size_t i = 0; i < Nodes.size(); i++)
    {
        auto node = Nodes[i];
        if (node->GetType() == ERenderNodeType::RenderResource)
        {
            // Plan the barriers for this resource, now that we have the pass ordering
            std::map<size_t, CTransition> transitions;
            for (const auto& pair : AdjLists[i])
            {
                size_t time = Nodes[pair.first]->_PassOrder;
                CTransition t;
                t.NodeId = i;
                t.StateDuring = pair.second->RequiredState;
                t.StateAfter = t.StateDuring;
                transitions.emplace(time, t);
            }
            auto iter = transitions.begin();
            while (true)
            {
                auto next = iter;
                std::advance(next, 1);
                if (next == transitions.end())
                    break;
                iter->second.StateAfter = next->second.StateDuring;
                iter = next;
            }
            // Actually store all those transitions
            for (const auto& tp : transitions)
                if (!tp.second.IsUnneeded())
                    Transitions[tp.first].push_back(tp.second);
        }
    }

    for (size_t i = 0; i < PassOrder.size(); i++)
    {
        size_t nodeId = PassOrder[i];
        std::cout << Nodes[nodeId]->GetName() << std::endl;
        for (const auto& tr : Transitions[i])
        {
            std::cout << Nodes[tr.NodeId]->GetName() << " " << (int)tr.StateDuring << " -> "
                      << (int)tr.StateAfter << std::endl;
        }
    }
}

std::shared_ptr<CResourceUsage> CRenderGraph::AddEdge(size_t src, size_t dst)
{
    auto usage = std::make_shared<CResourceUsage>();
    AdjLists[src][dst] = usage;
    AdjLists[dst][src] = usage;
    return usage;
}

bool CRenderGraph::CTransition::IsUnneeded() const { return StateDuring == StateAfter; }

}
