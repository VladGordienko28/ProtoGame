//-----------------------------------------------------------------------------
//	FxTypes.h: A basic FFX types
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 *	FFX synonyms
	 */
	using ShaderId = Int32;
	static const ShaderId INVALID_SHADER = -1;

	using TechniqueId = Int32;
	static const TechniqueId INVALID_TECHNIQUE = -1;

	/**
	 *	A technique
	 */
	struct Technique
	{
	public:
		static const SizeT MAX_TECHNIQUES = 16;

		String name;
		ShaderId shaderIds[rend::EShaderType::ST_MAX];

		friend IOutputStream& operator<<( IOutputStream& stream, const Technique& x )
		{
			stream << x.name << x.shaderIds;
			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, Technique& x )
		{
			stream >> x.name >> x.shaderIds;
			return stream;
		}
	};
}
}