/*=============================================================================
    FrALAudio.h: OpenAL audio class.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

//
// Audio constants.
//
#define AUDIO_MAX_FX				8
#define AUDIO_MAX_AMBIENT			4
#define AUDIO_STREAM_BUFFER_SIZE	65536
#define AUDIO_MAX_STREAM_BUFFERS	4


/*-----------------------------------------------------------------------------
    Audio structures.
-----------------------------------------------------------------------------*/

//
// A temporal audio FX effect.
//
struct TALFX
{
public:
	ALuint			iALId;
	FSound*			Sound;
	ALfloat			Gain;
	ALfloat			Pitch;
};


//
// An abstract music stream class.
//
class CMusicStreamBase
{
public:
	// CMusicStreamBase interface.
	virtual void Tick( Float Delta ) = 0;
	virtual void PlayMusic( FMusic* InMusic, Float FadeTime ) = 0;
};


//
// An scene ambient audio emitter.
//
struct TAmbientEmitter
{
public:
	FObject*		Owner;
	FSound*			Sound;
	Int32			iSource;
	ALfloat			Gain;
	ALfloat			Pitch;
	Float			RadiusSq;
	math::Vector	Position;
};


//
// Active scene ambient audio source.
//
struct TALAmbientSource
{
public:
	ALuint			iALId;
	Int32			iEmitter;
};


/*-----------------------------------------------------------------------------
    COpenALAudio.
-----------------------------------------------------------------------------*/

//
// An OpenAL audio.
//
class COpenALAudio: public CAudioBase, public CRefsHolder
{
public:
	// COpenALAudio interface.
	COpenALAudio();
	~COpenALAudio();

	// CAudioBase interface.
	void Flush();
	void Tick( Float Delta, FLevel* Scene );
	void PlayMusic( FMusic* Music, Float FadeTime );
	void PlayFX( FSound* Sound, Float Gain, Float Pitch );
	void PlayAmbient( FSound* Sound, math::Vector Location, Float Radius, Float Gain, Float Pitch, FObject* Owner );
	void StopAmbient( FObject* Owner );
	void FlushAmbients();

	// CRefsHolder interface.
	void CountRefs( CSerializer& S );

private:
	// AL variables.
	ALCcontext*			Context;
	ALCdevice*			Device;

	// Variables.
	TALFX					FXSources[AUDIO_MAX_FX];
	CMusicStreamBase*		Stream;
	Array<TAmbientEmitter>	Emitters;
	TALAmbientSource		AmbientSources[AUDIO_MAX_AMBIENT];

	// COpenALAudio interface.
	void RegisterSound( FSound* Sound );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/