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
		:	Resource( name ),
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

	void Layout::addRecreateCallback( UserLayout* userLayout, Callback callback )
	{
		assert( userLayout && callback );

		Listener listener;
		listener.userLayout = userLayout;
		listener.callback = callback;

		m_listeners.push( listener );
	}

	void Layout::removeRecreateCallback( UserLayout* userLayout )
	{
		assert( userLayout );

		for( Int32 i = 0; i < m_listeners.size(); )
		{
			if( m_listeners[i].userLayout == userLayout )
			{
				m_listeners.removeFast( i );
			}
			else
			{
				++i;
			}
		}
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
			for( const auto& it : m_listeners )
			{
				(( it.userLayout )->*( it.callback ))();
			}

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
