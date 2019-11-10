//-----------------------------------------------------------------------------
//	Element.h: An abstract UI element
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An abstrct UI element
	 */
	class Element: public NonCopyable
	{
	public:
		Element( String name, Root* root );
		virtual ~Element();
		virtual void load( JSon::Ptr node );

		virtual Bool eventActivate();
		virtual Bool eventDeactivate();
		virtual Bool eventResize();
		virtual Bool eventUpdate( Float delta );
		virtual Bool eventMouseDown( in::EMouseButton button, Float x, Float y );
		virtual Bool eventMouseUp( in::EMouseButton button, Float x, Float y );
		virtual Bool eventMouseMove( in::EMouseButton button, Float x, Float y );
		virtual Bool eventMouseEnter();
		virtual Bool eventMouseLeave();

		//virtual Bool eventDblClick( in::EMouseButton button, Float x, Float y );



		//virtual Bool eventMouseScroll( Int32 delta );
		//virtual Bool eventDragOver( void* data, Float x, Float y, Bool& accept );
		//virtual Bool eventDragDrop( void* data, Float x, Float y );
		//virtual Bool eventKeyDown( in::EKeyboardButton key, Bool repeat );
		//virtual Bool eventKeyUp( in::EKeyboardButton key );
		//virtual Bool eventCharType( Char typedChar );
		//virtual Bool eventGamepadDown( in::EGamepadButton button );
		//virtual Bool eventGamepadUp( in::EGamepadButton button );

		Bool isFocused() const;
		Bool isHighlighted() const;

		Float getWidth() const;
		void setWidth( Float width );

		Float getHeight() const;
		void setHeight( Float height );

		void setSize( Float width, Float height )
		{
			setWidth( width );
			setHeight( height );
		}

		Size getSize() const
		{
			return { getWidth(), getHeight() };
		}

		EHorizAlign getHorizAlign() const;
		void setHorizAlign( EHorizAlign horizAlign );

		EVertAlign getVertAlign() const;
		void setVertAlign( EVertAlign vertAlign );

		Float getMarginTop() const;
		void setMarginTop( Float value );

		Float getMarginLeft() const;
		void setMarginLeft( Float value );

		Float getMarginBottom() const;
		void setMarginBottom( Float value );

		Float getMarginRight() const;
		void setMarginRight( Float value );

		String getName() const
		{
			return m_name;
		}

		Bool isFocusable() const
		{
			return m_focusable;
		}

		Container* getOwner() const
		{
			return m_owner;
		}

		Root* getRoot() const
		{
			return m_root;
		}

	protected:
		// relation
		String m_name;
		Root* m_root;
		Container* m_owner;

		// layout
		Position m_position;
		Size m_size;
		Thickness m_margin;
		EHorizAlign m_horizAlign;
		EVertAlign m_vertAlign;

		// visual
		Float m_opacity; // todo: maybe move to sprite

		// animation
		Animator m_animator;

		// flags
		Bool m_focusable;

		Element() = delete;

		virtual void resizeInteral( const Size& parentSize, const Thickness& parentPadding );
		virtual void visit( IVisitor& visitor );
		virtual void draw( ICanvas& canvas );
		virtual void drawDebug( ICanvas& canvas );

		friend class Root;
		friend class Container;
		friend class Animator;

		// todo: get rid of it
		friend class RenderImpl;
	};
}
}