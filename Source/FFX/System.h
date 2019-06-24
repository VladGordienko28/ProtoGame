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
		void update();
		void shutdown();

		Effect::Ptr getEffect( String effectName, const rend::VertexDeclaration& vertexDeclaration );

	private:
		static constexpr const Char* SHADER_EXTENSION = TEXT( ".ffx" );

		struct CachedEffect
		{
			Effect* effect = nullptr;
			Int64 lastModificationTime = 0;
			Array<String> files;

			CachedEffect( Effect* inEffect, Int64 inTime, const Array<String>& inFiles )
				:	effect( inEffect ),
					lastModificationTime( inTime ),
					files( inFiles )
			{
				assert( effect );
				assert( lastModificationTime > 0 );
				assert( files.size() > 0 );
			}
		};

		String m_directory;
		Map<String, CachedEffect> m_effects;
		rend::Device* m_device;

		Effect::Ptr createEffect( String effectName, const rend::VertexDeclaration& vertexDeclaration );
	};
}
}