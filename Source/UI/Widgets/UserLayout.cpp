//-----------------------------------------------------------------------------
//	UserLayout.cpp: An element which contains widgets from the user's layout
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI/UI.h"

namespace flu
{
namespace ui
{
	UserLayout::UserLayout( String name, Root* root )
		:	Container( name, root ),
			m_layout( nullptr )
	{
	}

	UserLayout::~UserLayout()
	{
		unload();
	}

	Bool UserLayout::load( String resourceName )
	{
		unload();

		m_layout = res::ResourceManager::get<Layout>( resourceName, res::EFailPolicy::RETURN_NULL );

		if( m_layout.hasObject() )
		{
			m_layout->addRecreateCallback( this, ( Layout::Callback )( &UserLayout::recreate ) );
			recreate();
			return true;
		}
		else
		{
			error( L"Missing layout resource \"%s\"", *resourceName );
			return false;
		}
	}

	Bool UserLayout::unload()
	{
		destroyChildren();

		if( m_layout.hasObject() )
		{
			m_layout->removeRecreateCallback( this );
			m_layout = nullptr;
		}

		return true;
	}

	void UserLayout::recreate()
	{
		assert( m_layout.hasObject() );
		destroyChildren();

		JSon::Ptr elementArray = m_layout->getLayout()->getField( L"Children", JSon::EMissingPolicy::USE_NULL );
		if( elementArray.hasObject() && elementArray->isArray() )
		{
			loadChildren( elementArray );
		}
		else
		{
			error( L"Invalid layout \"%s\"", *m_layout->getName() );
		}
	}

	void UserLayout::destroyChildren()
	{
		for( auto& it : m_children )
		{
			delete it;
		}

		m_children.empty();	
	}
}
}