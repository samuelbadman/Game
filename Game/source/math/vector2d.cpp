#include "pch.h"
#include "vector2d.h"
#include "stringHelper.h"

std::string vector2d::toString() const
{
	return stringHelper::printf("[%f, %f]", x, y);
}
