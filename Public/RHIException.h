#pragma once
#include <stdexcept>

namespace Nome::RHI
{

class CRHIException : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

} /* namespace Nome::RHI */
