//-----------------------------------------------------------------------------
//	Container.h: An abstract UI container
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An abstrct UI container
	 */
	class Container: public Element
	{
	public:
		Container( String name, Root* root );
		~Container();
		void load( JSon::Ptr node ) override;

		virtual void addChild( Element* child );
		virtual void removeChild( Element* child );
		virtual void resizeChildren();
		Bool isChild( Element* child ) const;
		Element* getChildAt( Float x, Float y ) const;

		Bool eventResize() override;
		Bool eventUpdate( Float delta ) override;
		Bool eventMouseDown( in::EMouseButton button, Float x, Float y ) override;
		Bool eventMouseUp( in::EMouseButton button, Float x, Float y ) override;
		Bool eventMouseMove( in::EMouseButton button, Float x, Float y ) override;

		Float getPaddingTop() const;
		void setPaddingTop( Float value );

		Float getPaddingLeft() const;
		void setPaddingLeft( Float value );

		Float getPaddingBottom() const;
		void setPaddingBottom( Float value );

		Float getPaddingRight() const;
		void setPaddingRight( Float value );

	protected:
		Array<Element*> m_children;
		Thickness m_padding;

		void loadChildren( JSon::Ptr childrenArrayNode );
		void visit( IVisitor& visitor ) override;

		// todo: get rid of it
		friend class RenderImpl;
	};
}
}