//-----------------------------------------------------------------------------
//	FxCommon.h: A Fluorine Effect Framework common stuff
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 * A FFX framework version
	 */
	static const constexpr Char* FFX_VERSION = TXT( "FFX_1.0" );

	/**
	 * A constant buffer slot ids for various purposes
	 */
	enum EConstantBufferType
	{
		CBT_PerFrame = 0,
		CBT_PerView = 1,
		CBT_PerEffect = 2,
		CBT_MAX
	};
}
}