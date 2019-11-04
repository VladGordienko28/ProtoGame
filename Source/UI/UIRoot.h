//-----------------------------------------------------------------------------
//	UIRoot.h: An UI system Root
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An UI system root
	 */
	class Root final: public Container, public in::InputClient
	{
	public:
		using UPtr = UniquePtr<Root>;

		Root( rend::Device* device );
		~Root();

		void resize( Float width, Float height );
		void update( Float delta );
		void prepareBatches();
		void flushBatches();

		void setUIScale( Float newScale );
		Float getUIScale() const;

		Element* getFocused() const;
		void setFocused( Element* element );

		Element* getHighlighted() const;
		void setHighlighted( Element* element );

		Factory* getFactory() const
		{
			return m_factory.get();
		}

	private:
		Render::UPtr m_uiRender;
		Float m_uiScale;

		Factory::UPtr m_factory;

		Element* m_focused;
		Element* m_highlighted;

		Root() = delete;

		void moveFocus( EDirection direction );

		// InputClient
		Bool onMouseDown( in::EMouseButton button, Int32 x, Int32 y ) override;
		Bool onMouseUp( in::EMouseButton button, Int32 x, Int32 y ) override;
		Bool onMouseMove( in::EMouseButton button, Int32 x, Int32 y ) override;
		Bool onMouseDblClick( in::EMouseButton button, Int32 x, Int32 y ) override;
		Bool onMouseScroll( Int32 delta ) override;
		Bool onGamepadDown( in::GamepadId id, in::EGamepadButton button ) override;
		Bool onGamepadUp( in::GamepadId id, in::EGamepadButton button ) override;
		Bool onGamepadStick( in::GamepadId id, Int32 stick, const math::Vector& value ) override;
		Bool onGamepadTrigger( in::GamepadId id, Int32 trigger, Float value ) override;
		Bool onKeyboardDown( in::EKeyboardButton button, Bool repeat ) override;
		Bool onKeyboardUp( in::EKeyboardButton button ) override;
		Bool onKeyboardType( Char ch ) override;
	};
}
}