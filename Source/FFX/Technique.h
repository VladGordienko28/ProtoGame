//-----------------------------------------------------------------------------
//	Technique.h: A FFX technique
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 *	A techniue
	 */
	class Technique
	{
	public:
		AnsiString m_name;

		AnsiString m_vsEntryName;
		AnsiString m_psEntryName;

		rend::ShaderHandle m_vertexShader;
		rend::ShaderHandle m_pixelShader;


	private:

	};
}
}