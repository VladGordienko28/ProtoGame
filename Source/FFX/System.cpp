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
		for( auto& it : m_effects )
		{
			assert( it.value.effect );
			Effect* effect = it.value.effect;

			for( auto file : it.value.files )
			{
				String fileName = String::format( L"%s\\%s", *m_directory, *file );

				if( fm::fileExists( *fileName ) )
				{
					Int64 fileTime = fm::getFileModificationTime( *fileName );

					if( fileTime > it.value.lastModificationTime )
					{
						EffectLoadingContext context;

						effect->reload( context );
						it.value.lastModificationTime = fileTime;
						it.value.files = context.dependencies;
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
		String fileName = m_directory + effectName + SHADER_EXTENSION;

		if( !fm::fileExists( *fileName ) )
		{
			error( L"Shader %s is not found", *fileName );
			return nullptr;
		}

		Effect* effect = new Effect( effectName, fileName, vertexDeclaration, m_device );

		EffectLoadingContext context;

		if( effect && effect->load( context ) )
		{
			Int64 maxModificationTime = 0;

			for( auto it : context.dependencies )
			{
				maxModificationTime = max( maxModificationTime, fm::getFileModificationTime( *String::format( L"%s\\%s", *m_directory, *it ) ) );
			}

			m_effects.put( effectName, CachedEffect( effect, maxModificationTime, context.dependencies ) );
			return effect;
		}
		else
		{
			return nullptr;
		}
	}
}
}