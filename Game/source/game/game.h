#pragma once

#include "platform/graphics/meshResources.h"
#include "math/matrix4x4.h"
#include "platform/graphics/renderData.h"

class game
{
private:
	static bool running;
	static std::shared_ptr<class platformWindow> window;
	static std::shared_ptr<class graphics> graphicsEngine;
	static std::shared_ptr<class graphicsSurface> surface;
	static int64_t fps;
	static double ms;
	static bool graphicsInitialized;

	static sMeshResources triangleMeshResources;
	static matrix4x4 triangleWorldMatrix;
	static sRenderData triangleRenderData;
	static matrix4x4 viewProjectionMatrix;

public:
	static void start();
	static void exit();
	/* Called when the platform layer generates an input event.
	   inWindow acts as an id to identify the window that generated the event
	   inWindow will be nullptr when the input event comes from a gamepad as gamepad's are polled seperately from a window */
	static void onInputEvent(class platformWindow* inWindow, const struct sInputEvent& evt);
	static void onWindowMaximized(class platformWindow* inWindow, const struct sMaximizedEvent& evt);
	static void onWindowResized(class platformWindow* inWindow, const struct sResizedEvent& evt);
	static void onWindowMinimized(class platformWindow* inWindow, const struct sMinimizedEvent& evt);
	static void onWindowEnterSizeMove(class platformWindow* inWindow, const struct sEnterSizeMoveEvent& evt);
	static void onWindowExitSizeMove(class platformWindow* inWindow, const struct sExitSizeMoveEvent& evt);
	static void onWindowGainedFocus(class platformWindow* inWindow, const struct sGainedFocusEvent& evt);
	static void onWindowLostFocus(class platformWindow* inWindow, const struct sLostFocusEvent& evt);
	static void onWindowClosed(class platformWindow* inWindow, const struct sClosedEvent& evt);
	static void onWindowDestroyedEvent(class platformWindow* inWindow, const struct sDestroyedEvent& evt);
	static void onWindowEnterFullScreen(class platformWindow* inWindow, const struct sEnterFullScreenEvent& evt);
	static void onWindowExitFullScreen(class platformWindow* inWindow, const struct sExitFullScreenEvent& evt);

private:
	static void parseCommandLineArgs();
	static void initializeWindow();
	static void initializeGraphics();
	static void shutdownGraphics();
	static void initializeAudio();
	static void loadResources();

	static void begin();
	static void tick(float deltaSeconds);
	static void fixedTick(float fixedStep);
	static void render();

	static void updateViewProjectionMatrix();
};