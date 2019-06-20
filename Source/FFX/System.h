//-----------------------------------------------------------------------------
//	System.h: A FFX system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 *	A FFX system
	 */
	class System
	{
	public:
		using UPtr = UniquePtr<System>;

		System();
		~System();

		void init( rend::Device* device, String shadersDirectory );
		void shutdown();

		Effect::Ptr getEffect( String effectName );


	private:
		static constexpr const Char* SHADER_EXTENSION = TEXT( ".ffx" );

		rend::Device* m_device;

		String m_directory;



	};
}
}