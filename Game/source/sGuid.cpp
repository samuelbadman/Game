#include "pch.h"
#include "sGuid.h"
#include "math/mathLibrary.h"
#include "sString.h"

sGuid::sGuid()
{
    reset();
}

void sGuid::newGuid()
{
    a = mathLibrary::randomUInt32();
    b = mathLibrary::randomUInt32();
    c = mathLibrary::randomUInt32();
    d = mathLibrary::randomUInt32();
}

void sGuid::reset()
{
    a = 0;
    b = 0;
    c = 0;
    d = 0;
}

std::string sGuid::toString() const
{
    // Convert guid to string representation
    return sString::printf("%d,%d,%d,%d", a, b, c, d);
}
