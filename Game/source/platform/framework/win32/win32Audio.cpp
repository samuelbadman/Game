#include "pch.h"

#if defined(PLATFORM_WIN32)

#include "platform/framework/platformMessageBox.h"

static Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
static IXAudio2MasteringVoice* masterVoice = nullptr;

void platformInitAudio()
{
	HRESULT hr;

	if (FAILED(hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		platformMessageBoxFatal("xAudio2Audio::init failed to create xAudio2 instance.");
	}

	if (FAILED(hr = xAudio2->CreateMasteringVoice(&masterVoice)))
	{
		platformMessageBoxFatal("xAudio2Audio::init failed to create mastering voice.");
	}
}

void platformShutdownAudio()
{
	if (masterVoice)
	{
		masterVoice->DestroyVoice();
	}
	masterVoice = nullptr;
}

#endif // PLATFORM_WIN32
