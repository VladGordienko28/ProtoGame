//-----------------------------------------------------------------------------
//	System.cpp: A FFX system implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	System::System()
		:	m_device( nullptr )
	{
	}

	System::~System()
	{
	}

	void System::init( rend::Device* device, String shadersDirectory )
	{
		assert( device );
		assert( !m_device );
		assert( fm::directoryExists( *shadersDirectory ) );
		assert( fm::isAbsoluteFileName( *shadersDirectory ) );

		m_device = device;
		m_directory = shadersDirectory;
	}

	void System::shutdown()
	{
		m_device = nullptr;
		m_directory = L"";

		// todo: add auto removing from manager
		//assert( m_effects.isEmpty() );
	}

	// todo: move to the background thread
	void System::update()
	{
		for( auto& effect : m_effects )
		{
			assert( effect.value.effect );

			for( auto relativeFileName : effect.value.files )
			{
				String fileName = m_directory + relativeFileName;

				if( fm::fileExists( *fileName ) )
				{
					Int64 fileTime = fm::getFileModificationTime( *fileName );

					if( fileTime > effect.value.lastModificationTime )
					{
						reloadEffect( effect.value );
						break;
					}
				}	
			}
		}
	}

	Effect::Ptr System::getEffect( String effectName, const rend::VertexDeclaration& vertexDeclaration )
	{
		assert( m_device );
		assert( effectName );

		CachedEffect* effect = m_effects.get( effectName );

		if( effect )
		{
			assert( effect->effect );
			return effect->effect;
		}
		else
		{
			return createEffect( effectName, vertexDeclaration );
		}
	}

	Effect::Ptr System::createEffect( String effectName, const rend::VertexDeclaration& vertexDeclaration )
	{
		String relativeFileName = effectName + SHADER_EXTENSION;
		String fileName = m_directory + relativeFileName;

		if( !fm::fileExists( *fileName ) )
		{
			error( L"Shader \"%s\" is not found", *fileName );
			return nullptr;
		}

		CachedEffect cachedEffect;
		cachedEffect.effect = new Effect( effectName, vertexDeclaration, m_device );
		cachedEffect.relativeFileName = relativeFileName;

		if( cachedEffect.effect && reloadEffect( cachedEffect ) )
		{
			m_effects.put( effectName, cachedEffect );
			return cachedEffect.effect;
		}
		else
		{
			return nullptr;
		}
	}

	Bool System::reloadEffect( CachedEffect& cachedEffect )
	{
		assert( cachedEffect.effect );

		class IncludeProvider: public IIncludeProvider
		{
		public:
			IncludeProvider( String directory )
				:	m_directory( directory )
			{
			}

			Text::Ptr getInclude( String relativeFileName ) const override
			{
				if( !fm::isAbsoluteFileName( *relativeFileName ) )
				{
					return fm::readTextFile( *( m_directory + relativeFileName ) );				
				}
				else
				{
					return nullptr;
				}
			}

		private:
			String m_directory;
		};

		IncludeProvider includeProvider( m_directory );
		
		EffectLoadingContext context;
		context.relativeFileName = cachedEffect.relativeFileName;
		context.includeProvider = &includeProvider;

		Bool result = cachedEffect.effect->reload( context );

		if( result )
		{
			cachedEffect.files = context.dependencies;
		}

		Int64 lastModificationTime = 0;
		for( auto it : cachedEffect.files )
		{
			String fileName = m_directory + it;
			Int64 fileTime = fm::getFileModificationTime( *fileName );

			if( fileTime > lastModificationTime )
			{
				lastModificationTime = fileTime;
			}
		}

		cachedEffect.lastModificationTime = lastModificationTime;
		return result;
	}
}
}