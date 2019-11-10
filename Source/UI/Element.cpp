//-----------------------------------------------------------------------------
//	Element.cpp: An abstract UI element implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	Element::Element( String name, Root* root )
		:	m_name( name ),
			m_root( root ),
			m_owner( nullptr ),
			m_opacity( 1.f ),
			m_animator( this ),
			m_focusable( true )
	{
		assert( root );
	}

	Element::~Element()
	{
		if( m_root->getFocused() == this )
		{
			m_root->setFocused( m_owner );
		}

		if( m_root->getHighlighted() == this )
		{
			m_root->setHighlighted( nullptr );
		}
	}

	void Element::load( JSon::Ptr node )
	{
		assert( node.hasObject() );

		m_size.width = node->dotgetFloat( L"Size.W", 64.f );
		m_size.height = node->dotgetFloat( L"Size.H", 64.f );

		m_margin.top = node->dotgetFloat( L"Margin.T" );
		m_margin.left = node->dotgetFloat( L"Margin.L" );
		m_margin.bottom = node->dotgetFloat( L"Margin.B" );
		m_margin.right = node->dotgetFloat( L"Margin.R" );

		String horizAlignStr = node->dotgetString( L"HorizAlign", L"Left" );
		String vertAlignStr = node->dotgetString( L"VertAlign", L"Top" );

		m_horizAlign = horizAlignStr == L"Left" ? EHorizAlign::Left :
			horizAlignStr == L"Right" ? EHorizAlign::Right :
			horizAlignStr == L"Center" ? EHorizAlign::Center :
			horizAlignStr == L"Stretch" ? EHorizAlign::Stretch : EHorizAlign::Left;

		m_vertAlign = vertAlignStr == L"Top" ? EVertAlign::Top :
			vertAlignStr == L"Bottom" ? EVertAlign::Bottom :
			vertAlignStr == L"Center" ? EVertAlign::Center :
			vertAlignStr == L"Stretch" ? EVertAlign::Stretch : EVertAlign::Top;

		m_opacity = node->dotgetFloat( L"Opacity", 1.f );
	}

	Bool Element::eventActivate()
	{
		return false;
	}

	Bool Element::eventDeactivate()
	{
		return false;
	}

	Bool Element::eventResize()
	{
		return false;
	}

	Bool Element::eventUpdate( Float delta )
	{
		m_animator.update( delta );
		return true;
	}

	Bool Element::eventMouseDown( in::EMouseButton button, Float x, Float y )
	{
		m_root->setFocused( this );
		return false;
	}

	Bool Element::eventMouseUp( in::EMouseButton button, Float x, Float y )
	{
		return false;
	}

	Bool Element::eventMouseMove( in::EMouseButton button, Float x, Float y )
	{
		if( m_root->getHighlighted() != this )
		{
			if( m_root->getHighlighted() )
			{
				m_root->getHighlighted()->eventMouseLeave();
			}

			eventMouseEnter();
		}

		m_root->setHighlighted( this );
		return false;
	}

	Bool Element::eventMouseEnter()
	{
		return false;
	}

	Bool Element::eventMouseLeave()
	{
		return false;
	}

	void Element::resizeInteral( const Size& parentSize, const Thickness& parentPadding )
	{
		switch( m_horizAlign )
		{
			case EHorizAlign::Left:
				m_position.x = parentPadding.left + m_margin.left;
				break;

			case EHorizAlign::Right:
				m_position.x = parentSize.width - parentPadding.right - m_margin.right - m_size.width;
				break;

			case EHorizAlign::Center:
				m_position.x = 0.5f * ( parentSize.width - m_size.width ) + m_margin.left - m_margin.right;
				break;

			case EHorizAlign::Stretch:
				m_position.x = parentPadding.left + m_margin.left;
				m_size.width = parentSize.width - parentPadding.right - m_margin.right - m_position.x;
				break;

			default:
				assert( false );
				break;
		}

		switch( m_vertAlign )
		{
			case EVertAlign::Top:
				m_position.y = parentPadding.top + m_margin.top;
				break;

			case EVertAlign::Bottom:
				m_position.y = parentSize.height - parentPadding.bottom - m_margin.bottom - m_size.height;
				break;

			case EVertAlign::Center:
				m_position.y = 0.5f * ( parentSize.height - m_size.height ) + m_margin.top - m_margin.bottom;
				break;

			case EVertAlign::Stretch:
				m_position.y = parentPadding.top + m_margin.top;
				m_size.height = parentSize.height - parentPadding.bottom - m_margin.bottom - m_position.y;
				break;

			default:
				assert( false );
				break;
		}

		m_animator.setRefPose( m_position, m_size, m_opacity );
	}

	void Element::visit( IVisitor& visitor )
	{
		visitor.visit( this );
	}

	void Element::draw( ICanvas& canvas )
	{
	}

	void Element::drawDebug( ICanvas& canvas )
	{
	}

	Bool Element::isFocused() const
	{
		return m_root->getFocused() == this;
	}

	Bool Element::isHighlighted() const
	{
		return m_root->getHighlighted() == this;
	}

	Float Element::getWidth() const
	{
		return m_size.width;
	}

	void Element::setWidth( Float width )
	{
		if( m_horizAlign != EHorizAlign::Stretch )
		{
			m_size.width = width;

			if( m_owner )
			{
				m_owner->resizeChildren();
			}
		}
	}

	Float Element::getHeight() const
	{
		return m_size.height;
	}

	void Element::setHeight( Float height )
	{
		if( m_vertAlign != EVertAlign::Stretch )
		{
			m_size.height = height;

			if( m_owner )
			{
				m_owner->resizeChildren();
			}
		}
	}

	EHorizAlign Element::getHorizAlign() const
	{
		return m_horizAlign;
	}

	void Element::setHorizAlign( EHorizAlign horizAlign )
	{
		m_horizAlign = horizAlign;

		if( m_owner )
		{
			m_owner->resizeChildren();
		}
	}

	EVertAlign Element::getVertAlign() const
	{
		return m_vertAlign;
	}

	void Element::setVertAlign( EVertAlign vertAlign )
	{
		m_vertAlign = vertAlign;

		if( m_owner )
		{
			m_owner->resizeChildren();
		}
	}

	Float Element::getMarginTop() const
	{
		return m_margin.top;
	}

	void Element::setMarginTop( Float value )
	{
		m_margin.top = value;

		if( m_owner )
		{
			m_owner->resizeChildren();
		}
	}

	Float Element::getMarginLeft() const
	{
		return m_margin.left;
	}

	void Element::setMarginLeft( Float value )
	{
		m_margin.left = value;

		if( m_owner )
		{
			m_owner->resizeChildren();
		}
	}

	Float Element::getMarginBottom() const
	{
		return m_margin.bottom;
	}

	void Element::setMarginBottom( Float value )
	{
		m_margin.bottom = value;

		if( m_owner )
		{
			m_owner->resizeChildren();
		}
	}

	Float Element::getMarginRight() const
	{
		return m_margin.right;
	}

	void Element::setMarginRight( Float value )
	{
		m_margin.right = value;

		if( m_owner )
		{
			m_owner->resizeChildren();
		}
	}
}
}