#include "UniformBuffer.h"

namespace Nome::RHI
{

CUniformMemberDecl::CUniformMemberDecl(std::string keyword, std::string name)
    : HLSLKeyword(keyword), Name(name)
{
}

}
