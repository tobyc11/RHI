#pragma once
#include "RHIModuleAPI.h"
#include <stdexcept>

namespace RHI
{

class RHI_API CRHIException : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

class RHI_API CRHIRuntimeError : public CRHIException
{
public:
    CRHIRuntimeError(const char* msg)
        : CRHIException(msg)
    {
    }
};

} /* namespace RHI */
