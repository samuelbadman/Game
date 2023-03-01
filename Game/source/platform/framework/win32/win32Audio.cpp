#include "pch.h"

#include "platform/framework/platformMessageBox.h"

static Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
static IXAudio2MasteringVoice* masterVoice = nullptr;

namespace platformLayer
{
	void initAudio()
	{
		HRESULT hr;

		if (FAILED(hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		{
			platformLayer::messageBoxFatal("xAudio2Audio::init failed to create xAudio2 instance.");
		}

		if (FAILED(hr = xAudio2->CreateMasteringVoice(&masterVoice)))
		{
			platformLayer::messageBoxFatal("xAudio2Audio::init failed to create mastering voice.");
		}
	}

	void shutdownAudio()
	{
		if (masterVoice)
		{
			masterVoice->DestroyVoice();
		}
		masterVoice = nullptr;
	}
}
