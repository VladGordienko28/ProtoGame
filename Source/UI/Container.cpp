//-----------------------------------------------------------------------------
//	Container.h: An abstract UI container
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	Container::Container( String name, Root* root )
		:	Element( name, root ),
			m_children(),
			m_padding()
	{
		m_focusable = false;
	}

	Container::~Container()
	{
		for( auto& it : m_children )
		{
			delete it;
		}

		m_children.empty();
	}

	void Container::load( JSon::Ptr node )
	{
		Element::load( node );

		m_padding.top = node->dotgetFloat( L"Padding.T" );
		m_padding.left = node->dotgetFloat( L"Padding.L" );
		m_padding.bottom = node->dotgetFloat( L"Padding.B" );
		m_padding.right = node->dotgetFloat( L"Padding.R" );

		JSon::Ptr elementArray = node->getField( L"Children", JSon::EMissingPolicy::USE_NULL );
		if( elementArray.hasObject() && elementArray->isArray() )
		{
			loadChildren( elementArray );
		}
		else
		{
			error( L"Missing or invalid children array in \"%s\"", *m_name );
		}
	}

	Bool Container::eventResize()
	{
		resizeChildren();
		return true;
	}

	Bool Container::eventMouseDown( in::EMouseButton button, Float x, Float y )
	{
		Element* child = getChildAt( x, y );

		if( child )
		{
			return child->eventMouseDown( button, x - child->m_position.x, y - child->m_position.y );
		}
		else
		{
			return Element::eventMouseDown( button, x, y );
		}
	}

	Bool Container::eventMouseUp( in::EMouseButton button, Float x, Float y )
	{
		Element* child = getChildAt( x, y );

		if( child )
		{
			return child->eventMouseUp( button, x - child->m_position.x, y - child->m_position.y );
		}
		else
		{
			return Element::eventMouseUp( button, x, y );
		}
	}

	Bool Container::eventMouseMove( in::EMouseButton button, Float x, Float y )
	{
		Element* child = getChildAt( x, y );

		if( child )
		{
			return child->eventMouseMove( button, x - child->m_position.x, y - child->m_position.y );
		}
		else
		{
			return Element::eventMouseMove( button, x, y );
		}
	}

	void Container::addChild( Element* child )
	{
		assert( child );
		assert( !isChild( child ) );

		child->m_owner = this;
		m_children.push( child );
		resizeChildren();
	}

	void Container::removeChild( Element* child )
	{
		assert( child );
		assert( isChild( child ) );

		child->m_owner = nullptr;
		m_children.removeUnique( child, true );
		resizeChildren();
	}

	void Container::resizeChildren()
	{
		for( auto& it : m_children )
		{
			it->resizeInteral( m_size, m_padding );
			it->eventResize();
		}
	}

	Bool Container::eventUpdate( Float delta )
	{
		Element::eventUpdate( delta );

		for( auto& it : m_children )
		{
			it->eventUpdate( delta );
		}

		return true;
	}

	Bool Container::isChild( Element* child ) const
	{
		assert( child );

		return m_children.find( child ) != -1;
	}

	Element* Container::getChildAt( Float x, Float y ) const
	{
		for( auto& it : m_children )
		{
			if( ( x >= it->m_position.x ) && ( y >= it->m_position.y ) && 
				( x <= it->m_position.x + it->m_size.width ) && 
				( y <= it->m_position.y + it->m_size.height ) )
			{
				return it;
			}
		}

		return nullptr;
	}

	Float Container::getPaddingTop() const
	{
		return m_padding.top;
	}

	void Container::setPaddingTop( Float value )
	{
		m_padding.top = value;
		resizeChildren();
	}

	Float Container::getPaddingLeft() const
	{
		return m_padding.left;
	}

	void Container::setPaddingLeft( Float value )
	{
		m_padding.left = value;
		resizeChildren();
	}

	Float Container::getPaddingBottom() const
	{
		return m_padding.bottom;
	}

	void Container::setPaddingBottom( Float value )
	{
		m_padding.bottom = value;
		resizeChildren();
	}

	Float Container::getPaddingRight() const
	{
		return m_padding.right;
	}

	void Container::setPaddingRight( Float value )
	{
		m_padding.right = value;
		resizeChildren();
	}

	void Container::loadChildren( JSon::Ptr childrenArrayNode )
	{
		assert( childrenArrayNode.hasObject() && childrenArrayNode->isArray() );

		for( UInt32 i = 0; i < childrenArrayNode->arraySize(); ++i )
		{
			JSon::Ptr elementNode = childrenArrayNode->getElement( i, JSon::EMissingPolicy::USE_STUB );
			String elementType = elementNode->dotgetString( L"Type" );
			String elementName = elementNode->dotgetString( L"Name" );

			if( elementType && elementName )
			{
				Element* element = m_root->getFactory()->createElement( elementType, elementName, m_root );
					
				if( element )
				{
					element->load( elementNode );
					addChild( element );
				}
				else
				{
					error( L"Unknown layout element type \"%s\"", *elementType );
				}
			}
			else
			{
				error( L"Invalid %d-th layout element", i );
			}
		}
	}

	void Container::visit( IVisitor& visitor )
	{
		Element::visit( visitor );

		for( auto& it : m_children )
		{
			it->visit( visitor );
		}
	}
}
}