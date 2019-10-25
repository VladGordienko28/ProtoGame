//-----------------------------------------------------------------------------
//	WavFile.h: A wave file helpers and headers
//	Created by Vlad Gordienko, 2016
//-----------------------------------------------------------------------------

namespace flu
{
namespace aud
{
	/**
	 *	A struct that holds the RIFF data of the Wave file.
	 *	The RIFF data is the meta data information that holds,
	 *	the ID, size and format of the wave file.
	 */
	struct RiffHeader
	{
	public:
		AnsiChar chunkId[4];
		UInt32 chunkSize;
		AnsiChar format[4];

		Bool isValid() const
		{
			return	chunkId[0] == 'R' &&
					chunkId[1] == 'I' &&
					chunkId[2] == 'F' &&
					chunkId[3] == 'F' &&
					format[0] == 'W' &&
					format[1] == 'A' &&
					format[2] == 'V' &&
					format[3] == 'E';
		}
	};

	/**
	 *	A struct to hold fmt subchunk data for WAVE files
	 */
	struct WaveFormat
	{
	public:
		AnsiChar subChunkID[4];
		UInt32 subChunkSize;
		UInt16 audioFormat;
		UInt16 numChannels;
		UInt32 sampleRate;
		UInt32 byteRate;
		UInt16 blockAlign;
		UInt16 bitsPerSample;

		Bool isValid() const
		{
			return	subChunkID[0] == 'f' &&
					subChunkID[1] == 'm' &&
					subChunkID[2] == 't' &&
					subChunkID[3] == ' ';
		}

		ESoundFormat getSoundFormat() const
		{
			if( numChannels == 1 )
			{
				return bitsPerSample == 8 ? ESoundFormat::Mono8 : bitsPerSample == 16 ?
					ESoundFormat::Mono16 : ESoundFormat::Unknown;
			}
			else
			{
				return ESoundFormat::Unknown;
			}
		}
	};

	/**
	 *	A struct to hold the data of the wave file
	 */
	struct WaveData
	{
	public:
		AnsiChar subChunkID[4];
		UInt32 subChunk2Size;

		Bool isValid() const
		{
			return	subChunkID[0] == 'd' &&
					subChunkID[1] == 'a' &&
					subChunkID[2] == 't' &&
					subChunkID[3] == 'a';
		}
	};
}
}