#pragma once

// Loaded mesh graphics resource handles
struct sMeshResources
{
	size_t vertexBufferResourceHandle = 0;
	size_t vertexBufferViewHandle = 0;
	size_t indexBufferResourceHandle = 0;
	size_t indexBufferViewHandle = 0;
	uint32_t indexCount = 0;
};