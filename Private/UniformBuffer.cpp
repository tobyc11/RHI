#include "UniformBuffer.h"

namespace RHI
{

CUniformMemberDecl::CUniformMemberDecl(std::string keyword, std::string name)
    : HLSLKeyword(keyword), Name(name)
{
}

}
