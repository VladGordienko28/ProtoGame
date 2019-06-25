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
		static constexpr const Char* COMPILED_SHADER_EXTENSION = TEXT( ".ffxo" );

		struct CachedEffect
		{
			Effect* effect = nullptr;
			Int64 lastModificationTime = 0;
			String relativeFileName;
			Array<String> files;
		};

		String m_directory;
		Map<String, CachedEffect> m_effects;
		rend::Device* m_device;

		Compiler::UPtr m_compiler;

		Effect::Ptr createEffect( String effectName, const rend::VertexDeclaration& vertexDeclaration );
		Bool reloadEffect( CachedEffect& cachedEffect );
		Bool saveEffectToFile( String effectName, const Array<UInt8>& effectBlob );
	};
}
}