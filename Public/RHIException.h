#pragma once
#include <stdexcept>

namespace RHI
{

class CRHIException : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

class CRHIRuntimeError : public CRHIException
{
public:
    CRHIRuntimeError(const char* msg) : CRHIException(msg)
    {
    }
};

} /* namespace RHI */
