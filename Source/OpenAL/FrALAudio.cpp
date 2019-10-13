/*=============================================================================
    FrALAudio.cpp: OpenAL audio implementation.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "OpenALAud.h"

// Ogg & Vorbis include.
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

/*-----------------------------------------------------------------------------
    Wave file structures and routines.
-----------------------------------------------------------------------------*/

//
// Struct that holds the RIFF data of the Wave file.
// The RIFF data is the meta data information that holds,
// the ID, size and format of the wave file.
//
struct TRIFFHeader
{
public:
	// Fields.
	AnsiChar		chunkID[4];
	UInt32			chunkSize;
	AnsiChar		format[4];

	// Functions.
	Bool IsValid() const
	{
		return	chunkID[0] == 'R' &&
				chunkID[1] == 'I' &&
				chunkID[2] == 'F' &&
				chunkID[3] == 'F' &&
				format[0] == 'W' &&
				format[1] == 'A' &&
				format[2] == 'V' &&
				format[3] == 'E';
	}
};


//
// Struct to hold fmt subchunk data for WAVE files.
//
struct TWAVEFormat
{
public:
	// Fields.
	AnsiChar		subChunkID[4];
	UInt32			subChunkSize;
	UInt16			audioFormat;
	UInt16			numChannels;
	UInt32			sampleRate;
	UInt32			byteRate;
	UInt16			blockAlign;
	UInt16			bitsPerSample;

	// Functions.
	Bool IsValid() const
	{
		return	subChunkID[0] == 'f' &&
				subChunkID[1] == 'm' &&
				subChunkID[2] == 't' &&
				subChunkID[3] == ' ';
	}
	ALenum GetALFormat() const
	{
		if( numChannels == 1 )
			return	bitsPerSample == 8 ? AL_FORMAT_MONO8 :
					bitsPerSample == 16 ? AL_FORMAT_MONO16 : AL_INVALID;
		else if( numChannels == 2 )
			return	bitsPerSample == 8 ? AL_FORMAT_STEREO8 :
					bitsPerSample == 16 ? AL_FORMAT_STEREO16 : AL_INVALID;
		else
			return	AL_INVALID;
	}
};


//
// Struct to hold the data of the wave file.
//
struct TWAVEData
{
public:
	// Fields.
	AnsiChar		subChunkID[4];
	UInt32			subChunk2Size;

	// Functions.
	Bool IsValid() const
	{
		return	subChunkID[0] == 'd' &&
				subChunkID[1] == 'a' &&
				subChunkID[2] == 't' &&
				subChunkID[3] == 'a';
	}
};


/*-----------------------------------------------------------------------------
    CWavMusicStream implementation.
-----------------------------------------------------------------------------*/
#if 0

//
// Test wav file streamer.
//
class CWavMusicStream: public CMusicStreamBase, public CRefsHolder
{
public:
	// Music stream variables.
	FMusic*			ThisMusic;
	FMusic*			NextMusic;
	FILE*			Stream;
	ALfloat			Gain;
	Float			FadeInTime;
	Float			FadeOutTime;
	ALuint			Buffers[AUDIO_MAX_STREAM_BUFFERS];
	ALuint			iALSource;
	Byte*			Data;
	COpenALAudio*	ALAudio;

	// Wav specific.
	UInt32			FileSize;
	ALenum			WavFormat;
	UInt32			WavSize;
	UInt32			WavFrequency;
	UInt32			WavEntry;

	// CWavMusicStream interface.
	CWavMusicStream( COpenALAudio* InOpenALAudio );
	~CWavMusicStream();
	void LoadFromFile( const String& FileName );
	Bool ReadBlock( Int32 iBuff );

	// CMusicStreamBase interface.
	void Tick( Float Delta );
	void PlayMusic( FMusic* InMusic, Float FadeTime );

	// CRefsHolder interface.
	void CountRefs( CSerializer& S );
};


//
// Wav stream constructor.
//
CWavMusicStream::CWavMusicStream( COpenALAudio* InOpenALAudio )
	:	ALAudio( InOpenALAudio )
{
	// Allocate sources.
	alGenSources( 1, &iALSource );
	alSourcef( iALSource,	AL_PITCH,	1.f );
	alSourcef( iALSource,	AL_GAIN,	1.f );
	alSourcei( iALSource,	AL_LOOPING,	AL_FALSE );

	// Allocate sample buffers.
	alGenBuffers( AUDIO_MAX_STREAM_BUFFERS,	Buffers );

	// Allocate memory for all buffers.
	Data	= (Byte*)MemMalloc(AUDIO_STREAM_BUFFER_SIZE);
	
	// Stuff.
	FadeInTime		= 0.f;
	FadeOutTime		= 0.f;
	Gain			= 1.f;
	ThisMusic		= nullptr;
	NextMusic		= nullptr;
	Stream			= nullptr;
}


//
// Destroy the stream.
//
CWavMusicStream::~CWavMusicStream()
{
	// Stop playing.
	alSourceStop( iALSource );

	// Release AL objects.
	alDeleteSources( 1, &iALSource );
	alDeleteBuffers( AUDIO_MAX_STREAM_BUFFERS, Buffers );

	// Unload file.
	if( Stream )
	{
		fclose(Stream);
		Stream	= nullptr;
	}
	mem::free(Data);
}


//
// Start new music song.
//
void CWavMusicStream::PlayMusic( FMusic* InMusic, Float FadeTime )
{
	if( ThisMusic )
	{
		// Something played now.
		if( InMusic )
		{
			// Smoothly turn off old, and play new.
			NextMusic	= InMusic;
			Gain		= 1.f;
			FadeInTime	= 0.f;
			FadeOutTime	= FadeTime;
		}
		else
		{
			// Just turn off current music.
			NextMusic	= nullptr;
			Gain		= 1.f;
			FadeOutTime	= FadeTime;
		}
	}
	else
	{
		// Nothing played now.
		if( InMusic )
		{
			// Start new song immediately.
			ThisMusic	= InMusic;
			FadeInTime	= FadeTime;
			Gain		= 0.f;
			LoadFromFile( GDirectory+L"\\"+InMusic->FileName );
		}
	}
}


//
// Load initial wav data from file for track.
//
void CWavMusicStream::LoadFromFile( const String& FileName )
{
	// Stop song.
	alSourceStop( iALSource );

	// Close old file, if any.
	if( Stream )
	{
		fclose( Stream );
		Stream	= nullptr;
	}

	alSourceUnqueueBuffers( iALSource, AUDIO_MAX_STREAM_BUFFERS, &Buffers[0] );

	// Open the file.
	if( !GPlat->FileExists(FileName) )
	{
		log( L"MusicStream: File '%s' not found for '%s'", *FileName, *ThisMusic->GetName() );
		return;
	}

	Stream	= _wfopen( *FileName, L"rb" );
	{
		// File size.
		UInt32 OldPos = ftell( Stream );
		fseek( Stream, 0, SEEK_END );
		FileSize = ftell( Stream );
		fseek( Stream, OldPos, SEEK_SET );
	}

	// Read RIFF header.
	TRIFFHeader RiffHeader;
	fread( &RiffHeader, sizeof(TRIFFHeader), 1, Stream );
	if( !RiffHeader.IsValid() )
	{
		// Header corrupted.
		log( L"MusicStream: Invalid music '%s', file header corrupted", *ThisMusic->GetName() );
		return;
	}

	// Read in the 2nd chunk for the wave info.
	TWAVEFormat WaveFormat;
	fread( &WaveFormat, sizeof(TWAVEFormat), 1, Stream );
	if( !WaveFormat.IsValid() )
	{
		// Header corrupted.
		log( L"MusicStream: Invalid music file '%s', wave header corrupted", *ThisMusic->GetName() );
		return;
	}

	// Skip wav crap.
	if( WaveFormat.subChunkSize > 16 )
	{
		UInt16 tmp;
		fread( &tmp, sizeof(UInt16), 1, Stream );
	}

	TWAVEData	WaveData;
	fread( &WaveData, sizeof(TWAVEData), 1, Stream );
	if( !WaveData.IsValid() )
	{
		log( L"MusicStream: Invalid music file '%s', wave header corrupted", *ThisMusic->GetName() );
		return;
	}

	// Get track properties.
	WavEntry		= ftell(Stream);
	WavSize			= WaveData.subChunk2Size;
	WavFrequency	= WaveFormat.sampleRate;
	WavFormat		= WaveFormat.GetALFormat();


	// Initially read blocks.
	for( Int32 i=0; i<AUDIO_MAX_STREAM_BUFFERS; i++ )
	{
		ReadBlock( i );
		alSourceQueueBuffers( iALSource, 1, &Buffers[i] );
	}

	// Play!
	alSourcePlay(iALSource);
}


//
// Read a block of data from file to buffer return
// true if full block loaded and false if chunk of it.
//
Bool CWavMusicStream::ReadBlock( Int32 iBuff )
{
	Int32	Pos			= ftell(Stream);
	Int32	TotalSize	= Min<Int32>( AUDIO_STREAM_BUFFER_SIZE, WavSize-Pos );

	fread( Data, 1, TotalSize, Stream );

	if( TotalSize > 0 )
		alBufferData
		(
			Buffers[iBuff],
			WavFormat,
			Data,
			TotalSize,
			WavFrequency
		);

	return TotalSize == AUDIO_STREAM_BUFFER_SIZE;
}


//
// Update the music stream.
//
void CWavMusicStream::Tick( Float Delta )
{
	// Preload new chunks if any.
	ALint Processed;
	alGetSourcei( iALSource, AL_BUFFERS_PROCESSED, &Processed );

	while( Processed > 0 )
	{
		ALuint iALBuff;
		alSourceUnqueueBuffers( iALSource, 1, &iALBuff );

		Int32 i;
		for( i=0; i<AUDIO_MAX_STREAM_BUFFERS; i++ )
			if( iALBuff == Buffers[i] )
				break;

		if( ReadBlock( i ) )
		{
			// Normally play, add new chunk to queue.
			alSourceQueueBuffers( iALSource, 1, &iALBuff );
		}
		else
		{
			// Goto start of wav and play again.
			fseek( Stream, WavEntry, SEEK_SET );
			alSourceQueueBuffers( iALSource, 1, &iALBuff );
		}

		Processed--;
	}

	// Smoothly turn off old music.
	if( FadeOutTime != 0.f )
	{
		Gain	-= Delta/FadeOutTime;
		if( Gain <= 0.f )
		{
			alSourceStop( iALSource );
			ThisMusic	= nullptr;
			PlayMusic( NextMusic, FadeOutTime );
			FadeInTime	= FadeOutTime;
			FadeOutTime	= 0.f;
			Gain		= 0.f;
		}
	}

	// Smoothly turn on song.
	if( FadeInTime != 0.f )
	{
		Gain	+= Delta/FadeInTime;
		if( Gain > 1.f )
		{
			FadeInTime	= 0.f;
			Gain		= 1.f;
		}
	}

	// Set track gain.
	alSourcef( iALSource, AL_GAIN, Gain * ALAudio->MusicVolume * ALAudio->MasterVolume );
}


//
// Count references.
//
void CWavMusicStream::CountRefs( CSerializer& S )
{
	Serialize( S, ThisMusic );
	Serialize( S, NextMusic );

	if( !ThisMusic )
		FadeOutTime	= 1.f;
}

#endif
/*-----------------------------------------------------------------------------
    COggMusicStream implementation.
-----------------------------------------------------------------------------*/

//
// An Ogg music file streamer.
//
class COggMusicStream: public CMusicStreamBase, public CRefsHolder
{
public:
	// Music stream variables.
	FMusic*			ThisMusic;
	FMusic*			NextMusic;
	FILE*			Stream;
	ALfloat			Gain;
	Float			FadeInTime;
	Float			FadeOutTime;
	ALuint			Buffers[AUDIO_MAX_STREAM_BUFFERS];
	ALuint			iALSource;
	UInt8*			Data;
	COpenALAudio*	ALAudio;

	// Ogg/Vorbis specific.
	OggVorbis_File	OGGStream;
	vorbis_info		VorbisInfo;
	vorbis_comment	VorbisComment;
	ALenum			Format;

	// COggMusicStream interface.
	COggMusicStream( COpenALAudio* InOpenALAudio );
	~COggMusicStream();
	void LoadFromFile( const String& FileName );
	Bool ReadBlock( Int32 iBuff );

	// CMusicStreamBase interface.
	void Tick( Float Delta );
	void PlayMusic( FMusic* InMusic, Float FadeTime );

	// CRefsHolder interface.
	void CountRefs( CSerializer& S );
};


//
// Ogg stream constructor.
//
COggMusicStream::COggMusicStream( COpenALAudio* InOpenALAudio )
	:	ALAudio( InOpenALAudio )
{
	// Allocate sources.
	alGenSources( 1, &iALSource );
	alSourcef( iALSource,	AL_PITCH,	1.f );
	alSourcef( iALSource,	AL_GAIN,	1.f );
	alSourcei( iALSource,	AL_LOOPING,	AL_FALSE );

	// Allocate sample buffers.
	alGenBuffers( AUDIO_MAX_STREAM_BUFFERS,	Buffers );

	// Allocate memory for all buffers.
	Data	= (UInt8*)mem::malloc(AUDIO_STREAM_BUFFER_SIZE);

	// Stuff.
	FadeInTime		= 0.f;
	FadeOutTime		= 0.f;
	Gain			= 1.f;
	ThisMusic		= nullptr;
	NextMusic		= nullptr;
	Stream			= nullptr;
}


int CloseOgg( void* datasource )
{
	FILE* F	= (FILE*)datasource;
	return fclose(F);
}


long TellOgg( void* datasource )
{
	FILE* F	= (FILE*)datasource;
	return ftell(F);
}


int SeekOgg( void* datasource, ogg_int64_t offset, int whence )
{
	FILE* F	= (FILE*)datasource;
	return fseek( F, offset, whence );
}


size_t ReadOgg( void* ptr, size_t size, size_t nmemb, void* datasource )
{
	FILE* F	= (FILE*)datasource;
	return fread( ptr, size, nmemb, F );
}


//
// Load initial ogg data from file for track.
//
void COggMusicStream::LoadFromFile( const String& FileName )
{
	// Stop song.
	alSourceStop( iALSource );

	// Close old file, if any.
	if( Stream )
	{
		fclose( Stream );
		Stream	= nullptr;
	}

	alSourceUnqueueBuffers( iALSource, AUDIO_MAX_STREAM_BUFFERS, &Buffers[0] );

	// Open the file.
	if( !fm::fileExists( *FileName ) )
	{
		debug( L"MusicStream: File '%s' not found for '%s'", *FileName, *ThisMusic->GetName() );
		return;
	}
	_wfopen_s( &Stream, *FileName, L"rb" );

	// Ogg callbacks.
	ov_callbacks	Cb;
	Cb.close_func	= CloseOgg;
	Cb.tell_func	= TellOgg;
	Cb.seek_func	= SeekOgg;
	Cb.read_func	= ReadOgg;

	// Ogg file.
	if( ov_open_callbacks( Stream, &OGGStream, nullptr, 0, Cb ) )
	{
		debug( L"MusicStream: Could not open Ogg stream" );
		return;
	}

	VorbisInfo		= *ov_info( &OGGStream, -1 );
	VorbisComment	= *ov_comment( &OGGStream, -1 );
	Format			= VorbisInfo.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	// Initially read blocks.
	for( Int32 i=0; i<AUDIO_MAX_STREAM_BUFFERS; i++ )
	{
		ReadBlock( i );
		alSourceQueueBuffers( iALSource, 1, &Buffers[i] );
	}

	// Play!
	alSourcePlay(iALSource);
}


//
// Destroy the stream.
//
COggMusicStream::~COggMusicStream()
{
	// Stop playing.
	alSourceStop( iALSource );

	// Release AL objects.
	alDeleteSources( 1, &iALSource );
	alDeleteBuffers( AUDIO_MAX_STREAM_BUFFERS, Buffers );

	// Buffer's data.
	mem::free( Data );

	// Unload file.
	if( Stream )
	{
		fclose(Stream);
		Stream	= nullptr;
	}

	//ov_clear( &OGGStream ); // crashed!!
}


//
// Read a block of data from file to buffer return
// true if full block loaded and false if chunk of it.
//
Bool COggMusicStream::ReadBlock( Int32 iBuff )
{
	Int32 TotalSize = 0, Size;
	while( TotalSize < AUDIO_STREAM_BUFFER_SIZE )
	{
		Int32 Section;
		Size = ov_read
		(
			&OGGStream,
			(char*)(Data+TotalSize),
			AUDIO_STREAM_BUFFER_SIZE-TotalSize,
			0,
			2,
			1,
			&Section
		);

		if( Size == 0 )
			break;
		else if( Size < 0 )
			debug( L"MusicStream: Stream problem" );
		
		TotalSize += Size;
	}

	if( TotalSize > 0 )
		alBufferData
		(
			Buffers[iBuff],
			Format,
			Data,
			TotalSize,
			VorbisInfo.rate
		);

	return Size > 0;
}


//
// Update the music stream.
//
void COggMusicStream::Tick( Float Delta )
{
	// Preload new chunks if any.
	ALint Processed;
	alGetSourcei( iALSource, AL_BUFFERS_PROCESSED, &Processed );

	while( Processed > 0 )
	{
		ALuint iALBuff;
		alSourceUnqueueBuffers( iALSource, 1, &iALBuff );

		Int32 i;
		for( i=0; i<AUDIO_MAX_STREAM_BUFFERS; i++ )
			if( iALBuff == Buffers[i] )
				break;

		if( ReadBlock( i ) )
		{
			// Normally play, add new chunk to queue.
			alSourceQueueBuffers( iALSource, 1, &iALBuff );
		}
		else
		{
			// Goto start of wav and play again.
			ov_pcm_seek( &OGGStream, 0 );
			alSourceQueueBuffers( iALSource, 1, &iALBuff );
		}

		Processed--;
	}

	// Smoothly turn off old music.
	if( FadeOutTime > 0.f )
	{
		Gain	-= Delta/FadeOutTime;
		if( Gain <= 0.f )
		{
			alSourceStop( iALSource );
			ThisMusic	= nullptr;	
			PlayMusic( NextMusic, FadeOutTime );
			//FadeInTime	= FadeOutTime;
			//FadeOutTime	= 0.f;
			//Gain		= 0.f;
		}
	}

	// Smoothly turn on song.
	if( FadeInTime > 0.f )
	{
		Gain	+= Delta/FadeInTime;
		if( Gain > 1.f )
		{
			FadeInTime	= 0.f;
			Gain		= 1.f;
		}
	}

	// Set track gain.
	alSourcef( iALSource, AL_GAIN, Gain * ALAudio->MusicVolume * ALAudio->MasterVolume );
}


//
// Start new music song.
//
void COggMusicStream::PlayMusic( FMusic* InMusic, Float FadeTime )
{
	if( ThisMusic )
	{
		// Something played now.
		if( InMusic )
		{
			// Smoothly turn off old, and play new.
			NextMusic	= InMusic;
			Gain		= 1.f;
			FadeInTime	= 0.f;
			FadeOutTime	= FadeTime;
		}
		else
		{
			// Just turn off current music.
			NextMusic	= nullptr;
			Gain		= 1.f;
			FadeInTime	= 0.f;
			FadeOutTime	= FadeTime;
		}
	}
	else
	{
		// Nothing played now.
		if( InMusic )
		{
			// Start new song immediately.
			ThisMusic	= InMusic;
			FadeInTime	= FadeTime;
			FadeOutTime	= 0.f;
			Gain		= 0.f;
			LoadFromFile( fm::getCurrentDirectory() + L"\\"+InMusic->FileName );
		}
	}
}


//
// Count references.
//
void COggMusicStream::CountRefs( CSerializer& S )
{
	Serialize( S, ThisMusic );
	Serialize( S, NextMusic );

	if( !ThisMusic )
		FadeOutTime	= 1.f;
}


/*-----------------------------------------------------------------------------
    COpenALAudio implementation.
-----------------------------------------------------------------------------*/

//
// OpenAL audio system constructor.
//
COpenALAudio::COpenALAudio()
{
	// Open audio device.
	Device		= alcOpenDevice(nullptr);
	if( !Device )
	{
		fatal( L"No OpenAL device found" );
		return;
	}

	// Create audio context.
	Context		= alcCreateContext( Device, nullptr );
	if( !Context )
	{
		fatal( L"Failed create audio context" );
		return;
	}
	alcMakeContextCurrent( Context );

	// Default listener parameters.
	ALfloat DefaultPos[3] = { 0.f, 0.f, 0.f };
	ALfloat DefaultVel[3] = { 0.f, 0.f, 0.f };

	// Allocate OpenAL sources for FX.
	for( Int32 i=0; i<AUDIO_MAX_FX; i++ )
	{
		TALFX&	FX	= FXSources[i];

		alGenSources( 1, &FX.iALId );
		FX.Sound		= nullptr;
		FX.Gain			= 1.f;
		FX.Pitch		= 1.f;

		alSourcef	( FX.iALId,	AL_PITCH,		1.f );
		alSourcef	( FX.iALId,	AL_GAIN,		1.f );
		alSourcefv	( FX.iALId,	AL_POSITION,	DefaultPos );
		alSourcefv	( FX.iALId,	AL_VELOCITY,	DefaultVel );
		alSourcei	( FX.iALId,	AL_LOOPING,		AL_FALSE );
	}

	// Create soundtrack streamer.
	Stream	= new COggMusicStream( this );

	// Allocate OpenAL ambient sources.
	for( Int32 i=0; i<AUDIO_MAX_AMBIENT; i++ )
	{
		TALAmbientSource& Source = AmbientSources[i];

		alGenSources( 1, &Source.iALId );
		Source.iEmitter	= -1;

		alSourcef	( Source.iALId,	AL_PITCH,		1.f );
		alSourcef	( Source.iALId,	AL_GAIN,		1.f );
		alSourcefv	( Source.iALId,	AL_POSITION,	DefaultPos );
		alSourcefv	( Source.iALId,	AL_VELOCITY,	DefaultVel );
		alSourcei	( Source.iALId,	AL_LOOPING,		AL_TRUE );
	}

	// Default volume.
	MasterVolume	= 1.f;
	MusicVolume		= 1.f;
	FXVolume		= 1.f;

	// Setup listener.
	alListenerfv( AL_POSITION,		DefaultPos );
	alListenerfv( AL_VELOCITY,		DefaultVel );
	alListenerfv( AL_ORIENTATION,	DefaultPos );

	// Notify.
	info( L"OpenAL: OpenAL initialized" );
}


//
// OpenAL subsystem destructor.
//
COpenALAudio::~COpenALAudio()
{
	// Destroy stream.
	delete Stream;

	// Kill FX'es.
	for( Int32 i=0; i<AUDIO_MAX_FX; i++ )
	{
		alDeleteSources( 1, &FXSources[i].iALId );
	}

	// Kill all ambient sources.
	for( Int32 i=0; i<AUDIO_MAX_AMBIENT; i++ )
	{
		alDeleteSources( 1, &AmbientSources[i].iALId );
	}

	// Clean up all buffers being FSound's.
	if( GObjectDatabase )
		for( Int32 i=0; i<GObjectDatabase->GObjects.size(); i++ )
			if( GObjectDatabase->GObjects[i] && GObjectDatabase->GObjects[i]->IsA(FSound::MetaClass) )
			{
				FSound*	S = (FSound*)GObjectDatabase->GObjects[i];

				alDeleteBuffers( 1, (ALuint*)&S->AudioInfo );
				S->AudioInfo	= -1;
			}

	// And finally destroy device and its context.
	alcMakeContextCurrent( nullptr );
	alcDestroyContext( Context );
	alcCloseDevice( Device );
}


//
// Tick the audio subsystem.
//
void COpenALAudio::Tick( Float Delta, FLevel* Scene )
{ 
	// Validate volume.
	MasterVolume	= clamp( MasterVolume,	0.f, 1.f );
	FXVolume		= clamp( FXVolume,		0.f, 1.f );
	MusicVolume		= clamp( MusicVolume,	0.f, 1.f );

	// Update music streamer.
	Stream->Tick( Delta );  

	// Process scene's ambient sources.
	if( Scene )
	{
		TCamera&		Camera = Scene->Camera;
		math::Coords	Listener = math::Coords( Camera.Location, Camera.Rotation );

		// Turn on, or turn off ambient emitters.
		for( Int32 i=0; i<Emitters.size(); i++ )
		{
			TAmbientEmitter& E	= Emitters[i];

			// Remove?..
			if( E.iSource != -1 && (Camera.Location-E.Position).sizeSquared() > E.RadiusSq )
			{
				// This emitter is too far from listener now.
				alSourceStop( AmbientSources[E.iSource].iALId );
				AmbientSources[E.iSource].iEmitter	= -1;
				E.iSource	= -1;
			}

			// ..or add?
			if( E.iSource == -1 && (Camera.Location-E.Position).sizeSquared() <= E.RadiusSq )
			{
				// This emitter is close now.
				Int32 j;
				for( j=0; j<AUDIO_MAX_AMBIENT; j++ )
					if( AmbientSources[j].iEmitter == -1 )
						break;

				j	= clamp( j, 0, AUDIO_MAX_AMBIENT-1 );

				AmbientSources[j].iEmitter	= i;
				E.iSource					= j;
				alSourcei( AmbientSources[j].iALId, AL_BUFFER, E.Sound->AudioInfo );
				alSourcePlay( AmbientSources[j].iALId );
			}
		}

		// Process each active close emitters.
		for( Int32 i=0; i<AUDIO_MAX_AMBIENT; i++ )
		{
			TALAmbientSource& S = AmbientSources[i];
			if( S.iEmitter == -1 ) continue;

			TAmbientEmitter& E = Emitters[S.iEmitter];

			math::Vector LocalPos	= math::transformPointBy( E.Position, Listener );
			Float	DistFactor	= (1.f - LocalPos.size()/math::sqrt(E.RadiusSq));
			LocalPos.normalize();
			ALfloat ALPos[3] = { LocalPos.x, LocalPos.y, 0.f };

			alSourcefv	( S.iALId,	AL_POSITION,		ALPos );
			alSourcef	( S.iALId,	AL_PITCH,			E.Pitch );
			alSourcef	( S.iALId,	AL_GAIN,			E.Gain*DistFactor*MasterVolume*FXVolume );
		}
	}
}


//
// Stop the ambient emitter.
//
void COpenALAudio::StopAmbient( FObject* Owner )
{
	// If no appropriate actor, leave.
	if( !Owner )
		return;

	// Find emitter by it owner.
	for( Int32 i=0; i<Emitters.size(); i++ )
	{
		TAmbientEmitter& E = Emitters[i];

		if( E.Owner == Owner )
		{
			// Found.
			if( E.iSource != -1 )
			{
				// This emitter is playing now, unbind from
				// source.
				TALAmbientSource& S = AmbientSources[E.iSource];
				alSourceStop( S.iALId );
				S.iEmitter	= -1;
				E.iSource	= -1;
			}

			Emitters.removeFast(i);
			break;
		}
	}
}


//
// Add a new ambient sound emitter.
//
void COpenALAudio::PlayAmbient
( 
	FSound* Sound, 
	math::Vector Location, 
	Float Radius, 
	Float Gain, 
	Float Pitch, 
	FObject* Owner 
)
{
	// Validate.
	assert(Sound);
	Gain	= clamp( Gain,		0.f, 1.f );
	Radius	= clamp( Radius,	1.f, 1000.f );
	Pitch	= clamp( Pitch,		0.5f, 2.f );

	// Register FSound if any.
	if( Sound->AudioInfo == -1 )
		RegisterSound( Sound );

	// Don't add multiple times.
	if( Owner )
		for( Int32 i=0; i<Emitters.size(); i++ )
			if( Owner == Emitters[i].Owner )
				return;

	// Fill TAmbientEmitter struct.
	TAmbientEmitter E;
	E.Gain			= Gain;
	E.iSource		= -1;
	E.Owner			= Owner;
	E.Pitch			= Pitch;
	E.Position		= Location;
	E.RadiusSq		= Radius * Radius;
	E.Sound			= Sound;

	// Add it to list.
	Emitters.push( E );
}


//
// Count all references, used to stop destroyed
// sounds and cleanup wild pointers.
//
void COpenALAudio::CountRefs( CSerializer& S )
{
	// Check in Emitters.
	for( Int32 i=0; i<Emitters.size(); i++ )
	{
		TAmbientEmitter& E = Emitters[i];

		FSound* OldSound = E.Sound;
		Serialize( S, E.Sound );

		if( OldSound && !E.Sound )
		{
			if( E.iSource != -1 )
			{
				// This emitter is playing now, unbind from
				// source.
				TALAmbientSource& S = AmbientSources[E.iSource];
				alSourceStop( S.iALId );
				S.iEmitter	= -1;
				E.iSource	= -1;
			}

			Emitters.removeShift(i);
			i--;
		}
	}

	// Check in FX'es.
	for( Int32 i=0; i<AUDIO_MAX_FX; i++ )
	{
		TALFX& FX = FXSources[i];

		FSound* OldSound = FX.Sound;
		Serialize( S, FX.Sound );

		if( OldSound && !FX.Sound )
		{
			alSourceStop( FX.iALId );
			FX.Sound	= nullptr;
			FX.Pitch	= 0.f;
			FX.Gain		= 0.f;
		}
	}
}


//
// Remove all ambient emitters from the list. It's
// used when scenes changed.
//
void COpenALAudio::FlushAmbients()
{
	// Stop ambients.
	for( Int32 i=0; i<AUDIO_MAX_AMBIENT; i++ )
	{
		alSourceStop( AmbientSources[i].iALId );
		AmbientSources[i].iEmitter	= -1;
	}
	Emitters.empty();
}


//
// Reset entire audio subsystem.
//
void COpenALAudio::Flush()
{
	// Stop the soundtrack.
	Stream->PlayMusic( nullptr, 2.f );

	// Stop FX'es.
	for( Int32 i=0; i<AUDIO_MAX_FX; i++ )
	{
		TALFX& FX = FXSources[i];

		alSourceStop( FX.iALId );
		FX.Sound	= nullptr;
		FX.Pitch	= 0.f;
		FX.Gain		= 0.f;
	}

	// Stop ambients.
	for( Int32 i=0; i<AUDIO_MAX_AMBIENT; i++ )
	{
		alSourceStop( AmbientSources[i].iALId );
		AmbientSources[i].iEmitter	= -1;
	}
	Emitters.empty();

	// Kill all FSound's buffers.
	if( GProject )
		for( Int32 i=0; i<GProject->GObjects.size(); i++ )
			if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FSound::MetaClass) )
			{
				FSound* Sample	= As<FSound>(GProject->GObjects[i]);
				if( Sample->AudioInfo != -1 )
				{
					alDeleteBuffers( 1, (ALuint*)&Sample->AudioInfo );
					Sample->AudioInfo	= -1;
				}
			}
}


//
// Play a song.
//
void COpenALAudio::PlayMusic( FMusic* Music, Float FadeTime )
{
	Stream->PlayMusic( Music, FadeTime );
}


//
// Register a flu sound sample into OpenAL.
//
void COpenALAudio::RegisterSound( FSound* Sound )
{
	assert(Sound);

	// Don't register twice.
	if( Sound->AudioInfo != -1 )
		return;

	// Get audio data.
	UInt8* Data	= (UInt8*)Sound->GetData();
	assert(Data);

	// Read RIFF header.
	TRIFFHeader RiffHeader;
	mem::copy( &RiffHeader, Data, sizeof(TRIFFHeader) );
	Data	+= sizeof(TRIFFHeader);

	if( !RiffHeader.IsValid() )
	{
		// Header corrupted.
		debug( L"OpenAL: Failed register sound '%s', file header corrupted", *Sound->GetName() );
		return;
	}

	// Read in the 2nd chunk for the wave info.
	TWAVEFormat WaveFormat;
	mem::copy( &WaveFormat, Data, sizeof(TWAVEFormat) );
	Data	+= sizeof(TWAVEFormat);

	if( !WaveFormat.IsValid() )
	{
		// Header corrupted.
		debug( L"OpenAL: Failed register sound '%s', wave header corrupted", *Sound->GetName() );
		return;
	}

	// Skip wav unused crap.
	if( WaveFormat.subChunkSize > 16 )
		Data	+= sizeof(UInt16);

	// Skip useless data, until 'data' found.
	TWAVEData	WaveData;
	Int32		Timeout = 256;
	while( 1 )
	{
		mem::copy( &WaveData, Data, sizeof(TWAVEData) );

		if( !WaveData.IsValid() )
		{
			// Crap found.
			Data++;
			
			// Check for timeout.
			if( Timeout-- == 0 )
			{
				debug( L"OpenAL: Failed register sound '%s', missing wav data", *Sound->GetName() );
				return;
			}
		}
		else
		{
			// Good location.
			Data	+= sizeof(TWAVEData);
			break;
		}
	}

	// Get properties.
	ALsizei	Size		= WaveData.subChunk2Size;
	ALsizei	Frequency	= WaveFormat.sampleRate;
	ALenum	Format		= WaveFormat.GetALFormat();	

	// Create OpenAL buffer for this Sound object.
	alGenBuffers( 1, (ALuint*)&Sound->AudioInfo );
	alBufferData( Sound->AudioInfo, Format, Data, Size, Frequency );
}


//
// Play a temporal FX effect.
//
void COpenALAudio::PlayFX( FSound* Sound, Float Gain, Float Pitch )
{
	// Validate.
	assert(Sound);

	// Maybe sound buffer unregistered.
	if( Sound->AudioInfo == -1 )
		RegisterSound( Sound );

	// Try to find available FX slot.
	Int32 iSource = 0;
	for( Int32 i=0; i<AUDIO_MAX_FX; i++ )
	{
		ALint	State;
		alGetSourcei( FXSources[i].iALId, AL_SOURCE_STATE, &State );

		if( State != AL_PLAYING )
		{
			// Found.
			iSource	= i;
			break;
		}
	}

	// Setup slot.
	TALFX& FX	= FXSources[iSource];

	FX.Sound	= Sound;
	FX.Pitch	= Pitch;
	FX.Gain		= clamp( Gain, 0.f, 1.f );

	alSourcei( FX.iALId,	AL_BUFFER,	Sound->AudioInfo );		
	alSourcef( FX.iALId,	AL_PITCH,	FX.Pitch );
	alSourcef( FX.iALId,	AL_GAIN,	FX.Gain*MasterVolume*FXVolume );

	alSourcePlay( FX.iALId );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/