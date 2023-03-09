#pragma once

#include "graphicsApi.h"

class graphicsObject
{
private:
	eGraphicsApi api = eGraphicsApi::apiCount;

public:
	graphicsObject(const eGraphicsApi inApi);
	virtual ~graphicsObject() = default;

public:
	eGraphicsApi getApi() const { return api; }

	template<typename T>
	constexpr const T* as() const { return static_cast<const T*>(this); }
	template<typename T>
	constexpr T* as() { return static_cast<T*>(this); }
};