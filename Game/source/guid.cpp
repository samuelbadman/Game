#include "pch.h"
#include "guid.h"
#include "math/mathLibrary.h"
#include "string.h"

guid::guid()
{
    reset();
}

void guid::newGuid()
{
    a = mathLibrary::randomUInt32();
    b = mathLibrary::randomUInt32();
    c = mathLibrary::randomUInt32();
    d = mathLibrary::randomUInt32();
}

void guid::reset()
{
    a = 0;
    b = 0;
    c = 0;
    d = 0;
}

std::string guid::toString() const
{
    // Convert guid to string representation
    return sString::printf("%d,%d,%d,%d", a, b, c, d);
}
