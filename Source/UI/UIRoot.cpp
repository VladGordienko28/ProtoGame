//-----------------------------------------------------------------------------
//	UIRoot.cpp: An UI system Root implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	Root::Root( rend::Device* device )
		:	Container( L"UIRoot", this ),
			InputClient(),
			m_uiRender( Render::createRender( device ) ),
			m_uiScale( 1.f ),
			m_factory( new Factory() ),
			m_focused( nullptr ),
			m_highlighted( nullptr )
	{
		register_ui_product( getFactory(), Button );
	}

	Root::~Root()
	{
		m_uiRender = nullptr;
		m_factory = nullptr;
	}

	void Root::resize( Float width, Float height )
	{
		m_size.width = max( width, 0.f );
		m_size.height = max( height, 0.f );

		eventResize();
	}

	void Root::update( Float delta )
	{
		eventUpdate( delta );
	}

	void Root::prepareBatches()
	{
		m_uiRender->prepareBatches( this );
	}

	void Root::flushBatches()
	{
		m_uiRender->flushBatches();
	}

	void Root::setUIScale( Float newScale )
	{
		static const Float MAX_UI_SCALE = 10.f;

		m_uiScale = clamp( newScale, 1.f / MAX_UI_SCALE, MAX_UI_SCALE );
		eventResize();
	}

	Float Root::getUIScale() const
	{
		return m_uiScale;
	}

	Element* Root::getFocused() const
	{
		return m_focused;
	}

	void Root::setFocused( Element* element )
	{
		if( element != m_focused )
		{
			if( m_focused )
			{
				m_focused->eventDeactivate();
			}

			m_focused = element;

			if( element )
			{
				element->eventActivate();
			}
		}
	}

	Element* Root::getHighlighted() const
	{
		return m_highlighted;
	}

	void Root::setHighlighted( Element* element )
	{
		m_highlighted = element;
	}

	void Root::moveFocus( EDirection direction )
	{
		class FocusVisitor: public IVisitor
		{
		public:
			FocusVisitor( Array<Element*>& elements )
				:	m_elements( elements )
			{
			}

			void visit( Element* element ) override
			{
				if( element->isFocusable() )
				{
					m_elements.push( element );
				}
			}

		private:
			Array<Element*>& m_elements;
		};

		Array<Element*> focusableElements;
		FocusVisitor visitor( focusableElements );
		this->visit( visitor );

		// sort elements according to direction
		Float focusX = m_focused ? m_focused->m_position.x : 0.f;
		Float focusY = m_focused ? m_focused->m_position.y : 0.f;

		Element* bestElement = nullptr;
		Float bestWeight = 999999.9f;

		static const Float ADJACENT_WEIGHT = 0.0f;

		switch ( direction )
		{
			case EDirection::Up:
			{
				for( auto& it : focusableElements )
				{
					if( it != m_focused && focusY >= it->m_position.y )
					{
						Float weight = focusY - it->m_position.y;
						weight -= abs( it->m_position.x - focusX ) * ADJACENT_WEIGHT;
					
						if( weight < bestWeight )
						{
							bestWeight = weight;
							bestElement = it;
						}
					}
				}
				break;
			}
			case EDirection::Left:
			{
				for( auto& it : focusableElements )
				{
					if( it != m_focused && focusX >= it->m_position.x )
					{
						Float weight = focusX - it->m_position.x;
						weight -= abs( it->m_position.y - focusY ) * ADJACENT_WEIGHT;
					
						if( weight < bestWeight )
						{
							bestWeight = weight;
							bestElement = it;
						}
					}
				}
				break;
			}
			case EDirection::Right:
				for( auto& it : focusableElements )
				{
					if( it != m_focused && focusX <= it->m_position.x )
					{
						Float weight = it->m_position.x - focusX;
						weight -= abs( it->m_position.y - focusY ) * ADJACENT_WEIGHT;
					
						if( weight < bestWeight )
						{
							bestWeight = weight;
							bestElement = it;
						}
					}
				}
				break;

			case EDirection::Down:
				for( auto& it : focusableElements )
				{
					if( it != m_focused && focusY <= it->m_position.y )
					{
						Float weight = it->m_position.y - focusY;
						weight -= abs( it->m_position.x - focusX ) * ADJACENT_WEIGHT;
					
						if( weight < bestWeight )
						{
							bestWeight = weight;
							bestElement = it;
						}
					}
				}
				break;
		}

		setFocused( bestElement );
	}

	Bool Root::onMouseDown( in::EMouseButton button, Int32 x, Int32 y )
	{
		return eventMouseDown( button, x, y );
	}

	Bool Root::onMouseUp( in::EMouseButton button, Int32 x, Int32 y )
	{
		return eventMouseUp( button, x, y );
	}

	Bool Root::onMouseMove( in::EMouseButton button, Int32 x, Int32 y )
	{
		return eventMouseMove( button, x, y );
	}

	Bool Root::onMouseDblClick( in::EMouseButton button, Int32 x, Int32 y )
	{
		return 0;
	}

	Bool Root::onMouseScroll( Int32 delta )
	{return 0;
	}

	Bool Root::onGamepadDown( in::GamepadId id, in::EGamepadButton button )
	{
		////////////////////////////////////////
		if( button == in::EGamepadButton::GB_Up )
		{
			moveFocus( EDirection::Up );
		}
		if( button == in::EGamepadButton::GB_Left )
		{
			moveFocus( EDirection::Left );
		}
		if( button == in::EGamepadButton::GB_Right )
		{
			moveFocus( EDirection::Right );
		}
		if( button == in::EGamepadButton::GB_Down )
		{
			moveFocus( EDirection::Down );
		}
		
		
		return 0;
	}

	Bool Root::onGamepadUp( in::GamepadId id, in::EGamepadButton button )
	{return 0;
	}

	Bool Root::onGamepadStick( in::GamepadId id, Int32 stick, const math::Vector& value )
	{return 0;
	}

	Bool Root::onGamepadTrigger( in::GamepadId id, Int32 trigger, Float value )
	{return 0;
	}

	Bool Root::onKeyboardDown( in::EKeyboardButton button, Bool repeat )
	{
		////////////////////////////////////////
		if( button == in::EKeyboardButton::KB_Up )
		{
			moveFocus( EDirection::Up );
		}
		if( button == in::EKeyboardButton::KB_Left )
		{
			moveFocus( EDirection::Left );
		}
		if( button == in::EKeyboardButton::KB_Right )
		{
			moveFocus( EDirection::Right );
		}
		if( button == in::EKeyboardButton::KB_Down )
		{
			moveFocus( EDirection::Down );
		}

		
		
		return 0;


	}

	Bool Root::onKeyboardUp( in::EKeyboardButton button )
	{return 0;
	}

	Bool Root::onKeyboardType( Char ch )
	{return 0;
	}
}
}