//-----------------------------------------------------------------------------
//	Factory.h: An UI elements factory
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An UI elements factory
	 */
	class Factory: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<Factory>;
		using Constructor = Element*(*)( String name, Root* root );

		Factory()
			:	m_products()
		{
		}

		~Factory()
		{
			m_products.empty();
		}

		void registerProduct( String typeName, Constructor constructor )
		{
			assert( typeName && constructor );
			m_products.put( typeName, constructor );
		}

		Element* createElement( String typeName, String name, Root* root ) const
		{
			assert( typeName && name && root );
			Constructor const* constructor = m_products.get( typeName );

			if( constructor )
			{
				return (*constructor)( name, root );
			}
			else
			{
				return nullptr;
			}
		}

	private:
		Map<String, Constructor> m_products;
	};

	/**
	 *	A helper macro for elements registration
	 */
	#define register_ui_product( factory, elementName ) \
		factory->registerProduct( L#elementName, []( String name, Root* root )->Element* \
			{ \
				return new elementName( name, root ); \
			} );
}
}