//-----------------------------------------------------------------------------
//	SharedConstants.h: A fluorine engine specific shader's constants
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	An engine specific shaders shared constants
	 */
	class SharedConstants: public ffx::SharedConstants
	{
	public:
		// must be synced with Common.ffxh
		struct PerFrameData
		{
			Float gameTime;

			Float _padding[15]; // todo: eliminate it, since minimal size of constant buffer should be 16
		};

		struct PerViewData
		{
			Float viewProjectionMatrix[16];
			math::Vector4 worldCamera; // xy - location; zw - size
		};

		SharedConstants( rend::Device* device )
			:	ffx::SharedConstants( device )
		{
			initPerFrameBuffer( sizeof( PerFrameData ) );
			initPerViewBuffer( sizeof( PerViewData ) );
		}

		~SharedConstants()
		{
		}

		void setPerFrameData( const PerFrameData& data )
		{
			updatePerFrameBuffer( &data );
		}

		void setPerViewData( const PerViewData& data )
		{
			updatePerViewBuffer( &data );
		}
	};
}
}