#include "pch.h"
#include "vector2d.h"
#include "sString.h"

std::string vector2d::toString() const
{
	return sString::printf("[%f, %f]", x, y);
}
