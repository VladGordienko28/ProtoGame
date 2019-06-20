//-----------------------------------------------------------------------------
//	Effect.h: A FFX effect
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 *	A effect
	 */
	class Effect: public ReferenceCount
	{
	public:
		using Ptr = SharedPtr<Effect>;



		void apply();


	private:
		AnsiString m_name;
		Array<Technique> m_techniques;


	};
}
}