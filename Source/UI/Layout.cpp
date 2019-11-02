//-----------------------------------------------------------------------------
//	Layout.cpp: An UI layout resource implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	Layout::Layout( String name )
		:	m_name( name ),
			m_layout()
	{
	}

	Layout::~Layout()
	{
		m_layout = nullptr;
	}

	JSon::Ptr Layout::getLayout() const
	{
		return m_layout;
	}

	Bool Layout::create( const res::CompiledResource& compiledResource )
	{
		assert( compiledResource.isValid() );
		assert( !m_layout.hasObject() );

		BufferReader reader( compiledResource.data );
		String errorMsg;

		m_layout = JSon::loadFromStream( reader, &errorMsg );

		if( m_layout.hasObject() )
		{
			return true;
		}
		else
		{
			error( L"Unable to load UI layout \"%s\" with error %s",
				*m_name, *errorMsg );

			return false;
		}
	}

	void Layout::destroy()
	{
		m_layout = nullptr;
	}
}
}
