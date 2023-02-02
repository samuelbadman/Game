#include "pch.h"
#include "platform/graphics/graphicsApi.h"
#include "platform/graphics/direct3D12/direct3D12Graphics.h"
#include "platform/graphics/vulkan/vulkanGraphics.h"
#include "platform/framework/platformMessageBox.h"

static void(*shutdown)() = nullptr;
static void(*createSurface)(void*, uint32_t, uint32_t, std::shared_ptr<class graphicsSurface>&) = nullptr;
static void(*destroySurface)(std::shared_ptr<class graphicsSurface>&) = nullptr;
static void(*resizeSurface)(class graphicsSurface*, uint32_t, uint32_t) = nullptr;
static void(*render)(const uint32_t, const class graphicsSurface* const*, const bool, const uint32_t, const struct sMeshResources* const*) = nullptr;
static void(*loadMesh)(const size_t, const struct sVertexPos3Norm3Col4UV2* const, const size_t, const uint32_t* const, struct sMeshResources&) = nullptr;

static void setFunctionPointers(void(* const inShutdown)(),
	void(* const inCreateSurface)(void*, uint32_t, uint32_t, std::shared_ptr<class graphicsSurface>&),
	void(* const inDestroySurface)(std::shared_ptr<class graphicsSurface>&),
	void(* const inResizeSurface)(class graphicsSurface*, uint32_t, uint32_t),
	void(* const inRender)(const uint32_t, const class graphicsSurface* const*, const bool, const uint32_t, const struct sMeshResources* const*),
	void(* const inLoadMesh)(const size_t, const struct sVertexPos3Norm3Col4UV2* const, const size_t, const uint32_t* const, struct sMeshResources&))
{
	shutdown = inShutdown;
	createSurface = inCreateSurface;
	destroySurface = inDestroySurface;
	resizeSurface = inResizeSurface;
	render = inRender;
	loadMesh = inLoadMesh;
}

void graphicsInit(const eGraphicsApi graphicsApi, const bool softwareRenderer, const uint32_t backBufferCount)
{
	switch (graphicsApi)
	{
	case eGraphicsApi::direct3d12:
	{
		setFunctionPointers(&direct3d12Graphics::shutdown,
			&direct3d12Graphics::createSurface,
			&direct3d12Graphics::destroySurface,
			&direct3d12Graphics::resizeSurface,
			&direct3d12Graphics::render,
			&direct3d12Graphics::loadMesh);
		direct3d12Graphics::init(softwareRenderer, backBufferCount);
	}
	break;

	case eGraphicsApi::vulkan:
	{
		setFunctionPointers(&vulkanGraphics::shutdown,
			&vulkanGraphics::createSurface,
			&vulkanGraphics::destroySurface,
			&vulkanGraphics::resizeSurface,
			&vulkanGraphics::render,
			&vulkanGraphics::loadMesh);
		vulkanGraphics::init(false, backBufferCount);
	}
	break;

	default:
	{
		platformMessageBoxFatal("win32Graphics::graphicsInit: initializing unimplemented graphics api.");
	}
	break;
	}
}

void graphicsShutdown()
{
	shutdown();
}

void graphicsCreateSurface(void* hwnd, uint32_t width, uint32_t height, std::shared_ptr<class graphicsSurface>& outSurface)
{
	createSurface(hwnd, width, height, outSurface);
}

void graphicsDestroySurface(std::shared_ptr<class graphicsSurface>& surface)
{
	destroySurface(surface);
}

void graphicsResizeSurface(class graphicsSurface* surface, uint32_t width, uint32_t height)
{
	resizeSurface(surface, width, height);
}

void graphicsRender(const uint32_t numSurfaces, const class graphicsSurface* const* surfaces, const bool useVSync, const uint32_t meshCount, const struct sMeshResources* const* meshes)
{
	render(numSurfaces, surfaces, useVSync, meshCount, meshes);
}

void graphicsLoadMesh(const size_t vertexCount, const struct sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices,
	struct sMeshResources& outMeshResources)
{
	loadMesh(vertexCount, vertices, indexCount, indices, outMeshResources);
}
