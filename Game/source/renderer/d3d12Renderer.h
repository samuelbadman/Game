#pragma once

#include "renderer.h"

class d3d12Renderer : public renderer
{
public:
	virtual rendererPlatform getPlatform() const final { return rendererPlatform::direct3d12; }
	virtual bool init() final;
	virtual bool shutdown() final;
};