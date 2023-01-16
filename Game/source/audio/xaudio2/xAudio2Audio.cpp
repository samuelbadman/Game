#include "pch.h"
#include "xaudio2Audio.h"

static Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
static IXAudio2MasteringVoice* masterVoice = nullptr;

bool xAudio2Audio::init()
{
	HRESULT hr;

	if (FAILED(hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		return false;
	}

	if (FAILED(hr = xAudio2->CreateMasteringVoice(&masterVoice)))
	{
		return false;
	}

	return true;
}

void xAudio2Audio::destroyMasterVoice()
{
	if (masterVoice)
	{
		masterVoice->DestroyVoice();
	}
	masterVoice = nullptr;
}
