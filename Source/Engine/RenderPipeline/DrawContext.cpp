//-----------------------------------------------------------------------------
//	DrawContext.cpp:
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace gfx
{
	DrawContext::DrawContext( rend::Device* device )
		:	m_device( device ),
			m_sharedConstants( device )
	{
		assert( device );
	}

	DrawContext::~DrawContext()
	{
		assert( m_viewInfoStack.isEmpty() );
	}

	void DrawContext::pushViewInfo( const ViewInfo& info )
	{
		m_viewInfoStack.push( info );
		submitViewInfo( info );
	}

	void DrawContext::popViewInfo()
	{
		submitViewInfo( m_viewInfoStack.pop() );
	}

	const ViewInfo& DrawContext::getViewInfo() const
	{
		return m_viewInfoStack.peek();
	}

	void DrawContext::submitViewInfo( const ViewInfo& info )
	{
		SharedConstants::PerViewData perViewData;

		perViewData.worldCamera = math::Vector4( info.coords.origin, info.bounds.sizeX(), info.bounds.sizeY() );
		info.viewProjectionMatrix( backbufferWidth(), backbufferHeight(), perViewData.viewProjectionMatrix );

		m_sharedConstants.setPerViewData( perViewData );
	}
}
}