//-----------------------------------------------------------------------------
//	UIRoot.cpp: An UI system Root implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	Root::Root()
		:	Container(),
			m_uiScale( 1.f )
	{
		m_flatShadeEffect = res::ResourceManager::get<ffx::Effect>( L"System.Shaders.UI.FlatShade", res::EFailPolicy::FATAL );
	}

	Root::~Root()
	{
	}

	void Root::resize( Float width, Float height )
	{
		m_size.width = width;
		m_size.height = height;
	}

	void Root::update( Float delta )
	{
	}

	void Root::prepareBatches()
	{
	}

	void Root::flushBatches( rend::Device* device )
	{
		struct Vert
		{
			math::Vector pos;
			math::Color color;
		};

		Array<Vert> verts;

		for( auto& it : m_children )
		{
			Vert v;
			v.color = math::colors::RED;

			Float x, y, w, h;
			switch ( it->m_horizAlign )
			{
			case EHorizAlign::Left:
				x = 0 + it->m_margin.left;
				w = it->m_size.width;
				break;

			case EHorizAlign::Right:
				x = m_size.width - it->m_size.width - it->m_margin.right;
				w = it->m_size.width;
				break;

			case EHorizAlign::Center:
				x = (m_size.width - it->m_size.width) / 2.f;
				w = it->m_size.width;
				break;

			case EHorizAlign::Stretch:
				x = 0.f;
				w = m_size.width;
				break;
			}

			switch ( it->m_vertAlign )
			{
			case EVertAlign::Top:
				y = 0 + it->m_margin.top;
				h = it->m_size.height;
				break;

			case EVertAlign::Bottom:
				y = m_size.height - it->m_size.height - it->m_margin.bottom;
				h = it->m_size.height;
				break;

			case EVertAlign::Center:
				y = (m_size.height - it->m_size.height) / 2.f;
				h = it->m_size.height;
				break;

			case EVertAlign::Stretch:
				y = 0.f;
				h = m_size.height;
				break;
			}






			v.pos = { x, y };
			verts.push( v );

			v.pos = { x, y+h };
			verts.push( v );

			v.pos = { x+w, y };
			verts.push( v );



			v.pos = { x, y+h };
			verts.push( v );

			v.pos = { x+w, y };
			verts.push( v );

			v.pos = { x+w, y+h };
			verts.push( v );
		}


		if( m_vb == INVALID_HANDLE<rend::VertexBufferHandle>() )
		{
			m_vb = device->createVertexBuffer( sizeof(Vert), 9999, rend::EUsage::Dynamic, nullptr, "GUI_Vb" );
		}

		device->updateVertexBuffer( m_vb, &verts[0], verts.size() * sizeof(Vert) );

		m_flatShadeEffect->apply();

		device->setTopology( rend::EPrimitiveTopology::TriangleList );
		device->setVertexBuffer( m_vb );
		device->draw( verts.size(), 0 );
	}


/*

	class Root:: final: public Container, public in::InputClient
	{
	public:
		using UPtr = UniquePtr<Root>;





	private:
		Float m_uiScale;


	};*/


	void Root::setUIScale( Float newScale )
	{
		assert( false && "Not impelemented yet" );
		// dont forget to clamp scale!!
	}

	Float Root::getUIScale() const
	{
		return m_uiScale;
	}
}
}