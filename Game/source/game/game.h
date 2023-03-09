#pragma once

#include "platform/graphics/sMeshResources.h"
#include "math/matrix4x4.h"
#include "platform/graphics/sRenderData.h"

namespace platformLayer::window
{
	class platformWindow;

	struct sClosedEvent;
	struct sResizedEvent;
}

namespace platformLayer::input
{
	struct sInputEvent;
}

class graphics;
class graphicsSurface;

class game
{
private:
	static bool running;
	static std::shared_ptr<platformLayer::window::platformWindow> window;
	static std::shared_ptr<graphics> graphicsContext;
	static std::shared_ptr<graphicsSurface> surface;
	static int64_t fps;
	static double ms;
	static bool graphicsInitialized;

	static sMeshResources triangleMeshResources;
	static matrix4x4 triangleWorldMatrix;
	static sRenderData triangleRenderData;
	static matrix4x4 viewProjectionMatrix;

public:
	static void start();

private:
	static void parseCommandLineArgs();
	static void initializeWindow();
	static void initializeGamepad();
	static void initializeGraphics();
	static void shutdownGraphics();
	static void initializeAudio();
	static void loadResources();

	static void onWindowClosed(platformLayer::window::sClosedEvent&& evt);
	static void onWindowResized(platformLayer::window::sResizedEvent&& evt);
	static void onInput(platformLayer::input::sInputEvent&& evt);

	static void exit();
	static void begin();
	static void tick(float deltaSeconds);
	static void fixedTick(float fixedStep);
	static void render();

	static void updateViewProjectionMatrix();
};