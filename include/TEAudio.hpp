#pragma once

// Blendet Warnungen von 'xnamath.h' aus
#pragma warning( disable: 4838 )

#include "TEGlobal.hpp"

#define OUTPUT_CHANNELS 8
#define INPUT_CHANNELS 1

enum ReverbEffects {
	REVERB_FOREST,
	REVERB_DEFAULT,
	REVERB_GENERIC,
	REVERB_PADDEDCELL,
	REVERB_ROOM,
	REVERB_BATHROOM,
	REVERB_LIVINGROOM,
	REVERB_STONEROOM,
	REVERB_AUDITORIUM,
	REVERB_CONCERTHALL,
	REVERB_CAVE,
	REVERB_ARENA,
	REVERB_HANGAR,
	REVERB_CARPETHALLWAY,
	REVERB_HALLWAY,
	REVERB_STONECORRIDOR,
	REVERB_ALLEY,
	REVERB_CITY,
	REVERB_MOUNTAINS,
	REVERB_QUARRY,
	REVERB_PLAIN,
	REVERB_PARKINGLOT,
	REVERB_SEWERPIPE,
	REVERB_UNDERWATER,
	REVERB_SMALLROOM,
	REVERB_MEDIUMROOM,
	REVERB_LARGEROOM,
	REVERB_MEDIUMHALL,
	REVERB_LARGEHALL,
	REVERB_PLATE,
};

// Audiodatei bestehend aus Header (wfx) und Content (buffer)
typedef struct
{
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };
} TEAudioFile;

// Struktur-Prototypen (Nötig, da Klasse vor Struktur initialisiert werden muss
struct TEListener;
struct TEEmitter;

class TEAudio
{
public:
	TEAudio();
	~TEAudio();

	// Initialsisiert die Engine (true = erfolgreich, false = fehlerhaft (siehe Logfile))
	teResult init();
	// Lädt eine Datei in angegebene Struktur
	teResult loadSound(TEAudioFile& file, TCHAR* filename);
	// Spielt angegebene Datei ab
	teResult playSound(TEAudioFile& file, int source);
	// Spielt angegebene Datei mit angegebenem Emitter ab
	teResult playSound(TEAudioFile& file, TEEmitter& source);
	// Ändert Reverb in einen vordefinierten Arraywert
	teResult setSurrounding(int filter);
	// Tick angegebenen Emitter (Velocity und Position müssen vorher geändert werden
	teResult tickEmitter(TEEmitter& source);
	// Updatet die Koordinaten des Listeners (= Spieler), rotation muss normalisiert werden!
	teResult updateListener(D3DVECTOR posistion, D3DVECTOR velocity, D3DVECTOR rotation);
	// Ändert die Lautstärke des Spiels
	teResult setVolume(float volume);
	// Ändert die Lautstärke einer vordefinierten Source
	teResult setVolume(int source, float volume);
	// Ändert die Lautstärke eines TEEmitters
	void setVolume(TEEmitter& source, float volume);
	// Prüft ob angegebene Quelle/Emitter ein Geräusch abspielt
	bool isRunning(int source);
	bool isRunning(TEEmitter& source);

private:
	// Chunk in einer Datei suchen
	HRESULT FindChunk(HANDLE file, DWORD fourcc, DWORD & chunkSize, DWORD & chunkDataPosition);
	// Chunk in deiner Datei auslesen
	HRESULT ReadChunkData(HANDLE file, void * buffer, DWORD bufferSize, DWORD bufferOffset);

	// xAudio2-Instanz
	IXAudio2* xAudio2;
	// Mastering-Voice (Übergabe an Windows)
	IXAudio2MasteringVoice* masteringVoice;
	// Source-Voices {Musik, Menü}
	IXAudio2SourceVoice* sourceVoices[2];
	// Submix-Voice (notwendig für Effekte)
	IXAudio2SubmixVoice* submixVoice;
	// x3dAudio-Instanz
	X3DAUDIO_HANDLE x3dAudio;
	// true: TEAudio wurde initialisiert, false: TEAudio wurde nicht initialisiert
	bool active = false;


	// true = Subwoofer wird verwendet;
	bool useLFE;
	DWORD channelMask;
	// Anzahl an Audiokanälen
	UINT32 channelCount;
	// Aktuell ausgewählter Reverb-Effekt
	IUnknown* reverbEffect;
	// Einstellungen des DSPs (dititaler Signalprozessor)
	X3DAUDIO_DSP_SETTINGS dspSettings;
	// Nötig für Audiokanäle
	FLOAT32 matrixCoefficients[INPUT_CHANNELS * OUTPUT_CHANNELS];
	// Einstellungen, die zur 3D-Audio-Berechnung verwendet werden
	DWORD calcFlags;

	// = Camera/Spieler; maximal ein Listener möglich
	TEListener* listener;

	// Vordefinierte Reverb-Paramter
	const XAUDIO2FX_REVERB_I3DL2_PARAMETERS presets[30] =
	{
		XAUDIO2FX_I3DL2_PRESET_FOREST,
		XAUDIO2FX_I3DL2_PRESET_DEFAULT,
		XAUDIO2FX_I3DL2_PRESET_GENERIC,
		XAUDIO2FX_I3DL2_PRESET_PADDEDCELL,
		XAUDIO2FX_I3DL2_PRESET_ROOM,
		XAUDIO2FX_I3DL2_PRESET_BATHROOM,
		XAUDIO2FX_I3DL2_PRESET_LIVINGROOM,
		XAUDIO2FX_I3DL2_PRESET_STONEROOM,
		XAUDIO2FX_I3DL2_PRESET_AUDITORIUM,
		XAUDIO2FX_I3DL2_PRESET_CONCERTHALL,
		XAUDIO2FX_I3DL2_PRESET_CAVE,
		XAUDIO2FX_I3DL2_PRESET_ARENA,
		XAUDIO2FX_I3DL2_PRESET_HANGAR,
		XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY,
		XAUDIO2FX_I3DL2_PRESET_HALLWAY,
		XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR,
		XAUDIO2FX_I3DL2_PRESET_ALLEY,
		XAUDIO2FX_I3DL2_PRESET_CITY,
		XAUDIO2FX_I3DL2_PRESET_MOUNTAINS,
		XAUDIO2FX_I3DL2_PRESET_QUARRY,
		XAUDIO2FX_I3DL2_PRESET_PLAIN,
		XAUDIO2FX_I3DL2_PRESET_PARKINGLOT,
		XAUDIO2FX_I3DL2_PRESET_SEWERPIPE,
		XAUDIO2FX_I3DL2_PRESET_UNDERWATER,
		XAUDIO2FX_I3DL2_PRESET_SMALLROOM,
		XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM,
		XAUDIO2FX_I3DL2_PRESET_LARGEROOM,
		XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL,
		XAUDIO2FX_I3DL2_PRESET_LARGEHALL,
		XAUDIO2FX_I3DL2_PRESET_PLATE,
	};

public:
	static X3DAUDIO_DISTANCE_CURVE_POINT Emitter_LFE_CurvePoints[3];
	static X3DAUDIO_DISTANCE_CURVE Emitter_LFE_Curve;
	static X3DAUDIO_DISTANCE_CURVE_POINT Emitter_Reverb_CurvePoints[3];
	static X3DAUDIO_DISTANCE_CURVE Emitter_Reverb_Curve;
	static FLOAT32 emitterAzimuths[INPUT_CHANNELS];
};

typedef struct TEEmitter
{
	X3DAUDIO_EMITTER emitter;
	X3DAUDIO_CONE cone;
	IXAudio2SourceVoice* sourceVoice;

	TEEmitter()
	{
		emitter = { &cone, X3DAUDIO_VECTOR( 0, 0, 1 ), X3DAUDIO_VECTOR( 0, 1, 0 ), X3DAUDIO_VECTOR( 0, 0, 0 ), X3DAUDIO_VECTOR( 0, 0, 0 ), 2.0f, TE_PI / 4.0f, INPUT_CHANNELS, 1.0f, TEAudio::emitterAzimuths, (X3DAUDIO_DISTANCE_CURVE*)&X3DAudioDefault_LinearCurve, (X3DAUDIO_DISTANCE_CURVE*)&TEAudio::Emitter_LFE_Curve, NULL, NULL, (X3DAUDIO_DISTANCE_CURVE*)&TEAudio::Emitter_Reverb_Curve, 14.0f, 1.0f };
		cone = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f };
	}

	void update(X3DAUDIO_VECTOR pos, X3DAUDIO_VECTOR veloc, X3DAUDIO_VECTOR rot) {
		emitter.Position = pos;
		emitter.Velocity = veloc;
		emitter.OrientFront = rot;
	}
} TEEmitter;

typedef struct TEListener
{
	X3DAUDIO_LISTENER listener;
	X3DAUDIO_CONE cone;

	TEListener()
	{
		listener = { X3DAUDIO_VECTOR( 0, 0, 1 ), X3DAUDIO_VECTOR( 0, 1, 0 ), X3DAUDIO_VECTOR( 0, 0, 0 ), X3DAUDIO_VECTOR( 0, 0, 0 ), &cone };
		cone = { X3DAUDIO_PI*5.0f / 6.0f, X3DAUDIO_PI*11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };
	}

	void update(X3DAUDIO_VECTOR pos, X3DAUDIO_VECTOR veloc, X3DAUDIO_VECTOR rot) {
		listener.Position = pos;
		listener.Velocity = veloc;
		listener.OrientFront = rot;
	}
} TEListener;