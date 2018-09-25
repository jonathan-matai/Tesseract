#include "TEAudio.hpp"

TEAudio::TEAudio()
{
	xAudio2 = NULL;
	masteringVoice = NULL;

	// COM initialisieren
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	LOGFILE->print(TE_CONSTRUCTION, "Audio Engine wurde erstellt!");
}

TEAudio::~TEAudio()
{
	// Release Source Voices
		for (int i = 0; i < 2; i++) {
			if (sourceVoices[i]) {
				sourceVoices[i]->DestroyVoice();
				sourceVoices[i] = NULL;
			}
		}

	// Release Submix Voice
	if (submixVoice) {
		submixVoice->DestroyVoice();
		submixVoice = NULL;
	}

	// Release Mastering Voice
	if (masteringVoice) {
		masteringVoice->DestroyVoice();
		masteringVoice = NULL;
	}

	// Pointer releasen, COM uninitialisieren
	TE_SAFE_RELEASE(xAudio2);
	TE_SAFE_RELEASE(reverbEffect);
	CoUninitialize();

	LOGFILE->print(TE_CONSTRUCTION, "Audio Engine wurde beendet!");
}

teResult TEAudio::init()
{
	HRESULT hr;

	// xAudio2 initialisiren
	if (FAILED(hr = XAudio2Create(&xAudio2, 0)))
	{
		TRACE(true, "Konnte xAudio2 nicht initialisieren!");
		return false;
	}

	// Mastering Voice erstellen
	if (FAILED(hr = xAudio2->CreateMasteringVoice(&masteringVoice)))
	{
		TRACE(true, "Konnte Mastering Voice nicht erstellen!");
		TE_SAFE_RELEASE(xAudio2);
		return false;
	}

	// Kompatiblitaet ueberpruefen
	XAUDIO2_DEVICE_DETAILS details;
	if (FAILED(hr = xAudio2->GetDeviceDetails(0, &details)))
	{
		TRACE(true, "Nicht unterstütztes Audiogerät");
		TE_SAFE_RELEASE(xAudio2);
		return false;
	}

	// Audiokanaele überprüfen
	if ( details.OutputFormat.Format.nChannels > OUTPUT_CHANNELS)
	{
		TRACE(true, "Es können maxiaml 8 Kanäle verwendet werden!");
		TE_SAFE_RELEASE(xAudio2);
		return false;
	}

	channelCount = details.OutputFormat.Format.nChannels;

	// Reverb-Effekt erstellen
	if (FAILED(hr = XAudio2CreateReverb(&reverbEffect, 0)))
	{
		TRACE(true, "Reverb-Effekt konnte nicht erstellt werden!");
		TE_SAFE_RELEASE(xAudio2);
		return false;
	}

	// Effect Chain erstellen
	XAUDIO2_EFFECT_DESCRIPTOR effects[] = { { reverbEffect, TRUE, 1 } };
	XAUDIO2_EFFECT_CHAIN effectChain = { 1, effects };

	// Submix Voice erstellen
	if (FAILED(hr = xAudio2->CreateSubmixVoice(&submixVoice, 1, details.OutputFormat.Format.nSamplesPerSec, 0, 0, NULL, &effectChain)))
	{
		TRACE(true, "Submix Voice konnte nicht erstellt werden!");
		TE_SAFE_RELEASE(xAudio2);
		TE_SAFE_RELEASE(reverbEffect);
		return hr;
	}

	// Standardparameter für FX festlegen
	XAUDIO2FX_REVERB_PARAMETERS native;
	ReverbConvertI3DL2ToNative(&presets[5], &native);
	submixVoice->SetEffectParameters(0, &native, sizeof(native));

	// Setzt Flags zur 3D-Berechnung
	calcFlags = X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER
		| X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB
		| X3DAUDIO_CALCULATE_REVERB;

	// Prüft, ob Subwoofer verwendet wird
	useLFE = ((details.OutputFormat.dwChannelMask & SPEAKER_LOW_FREQUENCY) != 0);

	// Fügt ggf. LFE-Berechnung hinzu
	if(useLFE)
		calcFlags |= X3DAUDIO_CALCULATE_REDIRECT_TO_LFE;

	// x3DAudio initialisieren
	X3DAudioInitialize(details.OutputFormat.dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, x3dAudio);

	// Objekte erstellen
	listener = new TEListener();

	// Einstellung für digitalen Signalprozessor (DSP) erstellen
	dspSettings.SrcChannelCount = INPUT_CHANNELS;
	dspSettings.DstChannelCount = channelCount;
	dspSettings.pMatrixCoefficients = matrixCoefficients;

	LOGFILE->printf(TE_VALUE, "Using %i.%i Sound", channelCount - (int) useLFE, (int) useLFE);

	active = true;
	LOGFILE->print(TE_SUCCEEDED, "Audio Engine wurde erfolgreich initialisiert!");
	return true;
}

teResult TEAudio::loadSound(TEAudioFile & sound, TCHAR * filename)
{
	if (!active) {
		TRACE(false, "Engine muss initialisiert werden, bevor Sounds geladen werden können!");
		return false;
	}

	// Datei laden
	HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE == file) {
		char msg[128];
		sprintf_s(msg, "Datei %s konnte nicht geladen werden!", filename);
		TRACE(false, msg);
		return false;
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(file, 0, NULL, FILE_BEGIN)) {
		char msg[128];
		sprintf_s(msg, "Datei %s konnte nicht geladen werden!", filename);
		TRACE(false, msg);
		return false;
	}

	// Temporäre Variablen
	DWORD chunkSize;
	DWORD chunkPosition;
	DWORD fileType;

	// Riff-Chunk auslesen und laden
	FindChunk(file, 'FFIR', chunkSize, chunkPosition);
	ReadChunkData(file, &fileType, sizeof(DWORD), chunkPosition);

	if (fileType != 'EVAW') {
		char msg[128];
		sprintf_s(msg, "Datei %s ist keine WAVE-Datei!", filename);
		TRACE(false, msg);
		return false;
	}

	// Fmt-Chunk auslesen und laden
	FindChunk(file, ' tmf', chunkSize, chunkPosition);
	ReadChunkData(file, &sound.wfx, chunkSize, chunkPosition);

	// Data-Chunk auslesen und provisorisch in einen Buffer laden
	FindChunk(file, 'atad', chunkSize, chunkPosition);
	BYTE * pDataBuffer = new BYTE[chunkSize];
	ReadChunkData(file, pDataBuffer, chunkSize, chunkPosition);

	sound.buffer.AudioBytes = chunkSize;  // Groesse der Audiodatei
	sound.buffer.pAudioData = pDataBuffer;  // Buffer abspeichern
	sound.buffer.Flags = XAUDIO2_END_OF_STREAM; // Ende setzen

	return true;
}

teResult TEAudio::playSound(TEAudioFile & file, int source)
{
	if (!active) {
		TRACE(false, "Engine muss initialisiert werden, bevor Sounds abgespielt werden können!");
		return false;
	}

	if(source < 0 || source >= 2){
		char msg[128];
		sprintf_s(msg, "Source Voice %i ist nicht definiert!", source);
		TRACE(false, msg);
		return false;
	}

	HRESULT hr;

	if (FAILED(hr = xAudio2->CreateSourceVoice(&sourceVoices[source], (WAVEFORMATEX*)&file.wfx))) {
		TRACE(false, "Source Voice konnte nicht erstellt werden!");
		return false;
	}
	
	if (FAILED(hr = sourceVoices[source]->SubmitSourceBuffer(&file.buffer))) {
		TRACE(false, "Audiodatei konnte nicht an Source Voice übergeben werden!");
		return false;
	}
	
	if (FAILED(hr = sourceVoices[source]->Start(0))) {
		TRACE(false, "Audiodatei konnte nicht abgespielt werden!");
		return false;
	}
	
	return true;
}

teResult TEAudio::playSound(TEAudioFile & file, TEEmitter & source)
{
	if (!active) {
		TRACE(false, "Engine muss initialisiert werden, bevor Sounds abgespielt werden können!");
		return false;
	}

	HRESULT hr;

	// Sendet zu Mastering und Submix-Voice
	XAUDIO2_SEND_DESCRIPTOR sendDescriptors[2] = { { XAUDIO2_SEND_USEFILTER, masteringVoice }, { XAUDIO2_SEND_USEFILTER, submixVoice } };
	const XAUDIO2_VOICE_SENDS sendList = { 2, sendDescriptors };

	if (FAILED(hr = xAudio2->CreateSourceVoice(&source.sourceVoice, (WAVEFORMATEX*)&file.wfx, 0, 2.0f, NULL, &sendList))) {
		TRACE(false, "Source Voice konnte nicht erstellt werden!");
		return false;
	}

	if (FAILED(hr = source.sourceVoice->SubmitSourceBuffer(&file.buffer))) {
		TRACE(false, "Audiodatei konnte nicht an Source Voice übergeben werden!");
		return false;
	}

	if (FAILED(hr = source.sourceVoice->Start(0))) {
		TRACE(false, "Audiodatei konnte nicht abgespielt werden!");
		return false;
	}

	return true;
}

teResult TEAudio::setSurrounding(int filter)
{
	if (!active) {
		TRACE(false, "Engine muss initialisiert werden, bevor das Surrounding geändert werden kann!");
		return false;
	}

	if (filter < 0 || filter >= 30) {
		TRACE(false, "Reverb-Effekt konnte nicht geändert werden, da Index zu groß ist!");
		return false;
	}

	XAUDIO2FX_REVERB_PARAMETERS native;
	ReverbConvertI3DL2ToNative(&presets[filter], &native);
	submixVoice->SetEffectParameters(0, &native, sizeof(native));

	return false;
}

teResult TEAudio::tickEmitter(TEEmitter & source)
{
	if (!active) {
		TRACE(true, "Engine muss initialisiert werden, bevor Emitter getickt werden können!");
		return false;
	}

	if (!source.sourceVoice)
		return false;

	// Führt Berechnungen durch
	X3DAudioCalculate(x3dAudio, &listener->listener, &source.emitter, calcFlags, &dspSettings);

	// Wendet Berechnungen an
	source.sourceVoice->SetFrequencyRatio(dspSettings.DopplerFactor);
	source.sourceVoice->SetOutputMatrix(masteringVoice, INPUT_CHANNELS, channelCount, matrixCoefficients);
	source.sourceVoice->SetOutputMatrix(submixVoice, 1, 1, &dspSettings.ReverbLevel);

	XAUDIO2_FILTER_PARAMETERS paramDirect = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient), 1.0f }; // see XAudio2CutoffFrequencyToRadians() in XAudio2.h for more information on the formula used here
	source.sourceVoice->SetOutputFilterParameters(masteringVoice, &paramDirect);
	XAUDIO2_FILTER_PARAMETERS paramReverb = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFReverbCoefficient), 1.0f }; // see XAudio2CutoffFrequencyToRadians() in XAudio2.h for more information on the formula used here
	source.sourceVoice->SetOutputFilterParameters(submixVoice, &paramReverb);

	return true;
}

teResult TEAudio::updateListener(D3DVECTOR posistion, D3DVECTOR velocity, D3DVECTOR rotation)
{
	if (!active) {
		TRACE(true, "Engine muss initialisiert werden, bevor Listener geändert werden können!");
		return false;
	}

	listener->update(posistion, velocity, rotation);
	return true;
}

teResult TEAudio::setVolume(float volume)
{
	if (!active) {
		TRACE(false, "Engine muss initialisiert werden, bevor Lautstärke geändert werden kann!");
		return false;
	}

	masteringVoice->SetVolume(volume);
	return true;
}

teResult TEAudio::setVolume(int source, float volume)
{
	if (!active) {
		TRACE(false, "Engine muss initialisiert werden, bevor Lautstärke geändert werden kann!");
		return false;
	}

	if(source < 0 || source >= 2){
		char msg[128];
		sprintf_s(msg, "Source Voice %i ist nicht definiert!", source);
		TRACE(false, msg);
		return false;
	}

	if (!sourceVoices[source]) {
		char msg[128];
		sprintf_s(msg, "Source Voice %i ist NULL!", source);
		TRACE(false, msg);
	}

	sourceVoices[source]->SetVolume(volume);
	return true;
}

void TEAudio::setVolume(TEEmitter & source, float volume)
{
	source.sourceVoice->SetVolume(volume);
}

bool TEAudio::isRunning(int source)
{
	if (source < 0 || source >= 2) {
		char msg[128];
		sprintf_s(msg, "Source Voice %i ist nicht definiert!", source);
		TRACE(false, msg);
		return false;
	}

	if (!sourceVoices[source]) {
		char msg[128];
		sprintf_s(msg, "Source Voice %i ist NULL", source);
		TRACE(false, msg);
		return false;
	}

	XAUDIO2_VOICE_STATE state;
	sourceVoices[source]->GetState(&state);
	return state.BuffersQueued > 0 ? true : false;
}

bool TEAudio::isRunning(TEEmitter & source)
{
	XAUDIO2_VOICE_STATE state;
	source.sourceVoice->GetState(&state);
	return state.BuffersQueued > 0 ? true : false;
}

HRESULT TEAudio::FindChunk(HANDLE file, DWORD fourcc, DWORD & chunkSize, DWORD & chunkDataPosition)
{
	HRESULT hr = S_OK;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(file, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK)
	{
		DWORD dwRead;

		if (0 == ReadFile(file, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (0 == ReadFile(file, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		switch (dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;

			if (0 == ReadFile(file, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(file, dwChunkDataSize, NULL, FILE_CURRENT))
				return HRESULT_FROM_WIN32(GetLastError());
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc)
		{
			chunkSize = dwChunkDataSize;
			chunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize)
			return S_FALSE;
	}

	return S_OK;
}

HRESULT TEAudio::ReadChunkData(HANDLE file, void * buffer, DWORD bufferSize, DWORD bufferOffset)
{
	HRESULT hr = S_OK;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(file, bufferOffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwRead;

	if (0 == ReadFile(file, buffer, bufferSize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());

	return hr;
}

X3DAUDIO_DISTANCE_CURVE_POINT TEAudio::Emitter_LFE_CurvePoints[] = { 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f };
X3DAUDIO_DISTANCE_CURVE TEAudio::Emitter_LFE_Curve = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_LFE_CurvePoints[0], 3 };
X3DAUDIO_DISTANCE_CURVE_POINT TEAudio::Emitter_Reverb_CurvePoints[] = { 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, 0.0f };
X3DAUDIO_DISTANCE_CURVE TEAudio::Emitter_Reverb_Curve = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_Reverb_CurvePoints[0], 3 };
FLOAT32 TEAudio::emitterAzimuths[] = {};