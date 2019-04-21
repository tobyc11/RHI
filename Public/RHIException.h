#pragma once
#include "RHIModuleAPI.h"
#include <stdexcept>
#include <string>

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

    CRHIRuntimeError(const std::string& msg)
        : CRHIException(msg.c_str())
    {
    }
};

} /* namespace RHI */
