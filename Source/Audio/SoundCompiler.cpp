//-----------------------------------------------------------------------------
//	SoundCompiler.cpp: A sound files compiler implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Audio.h"

namespace flu
{
namespace aud
{
	SoundCompiler::SoundCompiler()
	{
	}

	SoundCompiler::~SoundCompiler()
	{
	}

	Bool SoundCompiler::compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
		res::CompilationOutput& output ) const
	{
		auto loader = dependencyProvider.getBinaryFile( relativePath );
		assert( loader.hasObject() );
	
		RiffHeader riffHeader;
		loader->readData( &riffHeader, sizeof( RiffHeader ) );

		if( !riffHeader.isValid() )
		{
			output.errorMsg = String::format( TXT( "\"%s\" is not a wav file, bad riff header" ), *relativePath );
			return false;
		}

		WaveFormat waveFormat;
		loader->readData( &waveFormat, sizeof( WaveFormat ) );

		if( !waveFormat.isValid() )
		{
			output.errorMsg = String::format( TXT( "\"%s\" is not a wav file, bad wave format header" ), *relativePath );
			return false;
		}

		// skip unused optional wav stuff
		if( waveFormat.subChunkSize > 16 )
		{
			UInt16 unused;
			loader->readData( &unused, sizeof( UInt16 ) );
		}

		// skip useless wav data, until 'data' found
		WaveData waveData;
		UInt32 timeout = 256; // bytes

		while( true )
		{
			SizeT from = loader->tell();
			loader->readData( &waveData, sizeof( WaveData ) );
	
			if( waveData.isValid() )
			{
				// found
				break;
			}
			else
			{
				// continue searching
				if( timeout-- == 0 )
				{
					output.errorMsg = String::format( TXT( "\"%s\" missing wave data" ), *relativePath );
					return false;
				}

				loader->seek( from + 1 ); // skip one byte
			}
		}

		// only mono sounds allowed
		if( waveFormat.numChannels != 1 )
		{
			output.errorMsg = TXT( "Only mono wave files allowed as Sound" );
			return false;
		}

		// only non-compressed(PCM) sounds allowed
		if( waveFormat.audioFormat != 1 )
		{
			output.errorMsg = TXT( "Compressed wave files is not supported" );
			return false;
		}

		UInt32 waveSize = waveData.subChunk2Size;
		UInt32 waveFrequency = waveFormat.sampleRate;
		ESoundFormat soundFormat = waveFormat.getSoundFormat();

		if( soundFormat == ESoundFormat::Unknown )
		{
			output.errorMsg = TXT( "Unknown wave format" );
			return false;
		}

		UserBufferWriter writer( output.compiledResource.data );

		writer << waveSize;
		writer << waveFrequency;
		writer << soundFormat;

		void* dataToWrite = writer.reserveData( waveSize );
		assert( dataToWrite );

		SizeT dataBytesRead = loader->readData( dataToWrite, waveSize );
		if( dataBytesRead != waveSize )
		{
			output.errorMsg = TXT( "Unexpected end of wave data" );
			return false;
		}

		return true;
	}
}
}