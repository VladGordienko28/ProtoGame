/*=============================================================================
    FrEdTic.cpp: Editor tick function.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    CEditor implementation.
-----------------------------------------------------------------------------*/

//
// Editor tick.
//
void CEditor::Tick( Float Delta )
{
	profile_zone( EProfilerGroup::General, TotalTime );

	profile_counter( EProfilerGroup::Memory, Allocated_Kb, mem::stats().totalAllocatedBytes / 1024 );

	// GPU memory stats
	profile_counter( EProfilerGroup::GPUMemory, GPU_Total_Kb,		g_device->getMemoryStats().totalBytes() / 1024 );
	profile_counter( EProfilerGroup::GPUMemory, GPU_Texture_Kb,		g_device->getMemoryStats().m_texureBytes / 1024 );
	profile_counter( EProfilerGroup::GPUMemory, GPU_Vertex_Kb,		g_device->getMemoryStats().m_vertexBufferBytes / 1024 );
	profile_counter( EProfilerGroup::GPUMemory, GPU_Constant_Kb,	g_device->getMemoryStats().m_constantBufferBytes / 1024 );
	profile_counter( EProfilerGroup::GPUMemory, GPU_Index_Kb,		g_device->getMemoryStats().m_indexBufferBytes / 1024 );

	// Draw calls stats
	profile_counter( EProfilerGroup::DrawCalls, Draw_Calls, g_device->getDrawStats().m_drawCalls );
	profile_counter( EProfilerGroup::DrawCalls, State_Switchings, g_device->getDrawStats().m_blendStateSwitches );

	// Get active page.
	WEditorPage* Active = (WEditorPage*)EditorPages->GetActivePage();
	
	{
		profile_zone( EProfilerGroup::General, TickTime );

		// Tick active page.
		if( Active )
			Active->TickPage( Delta );

		// Update audio.
		if( Active && Active->PageType == PAGE_Play )
			GAudio->Tick( Delta, ((WPlayPage*)Active)->PlayLevel );
		else
			GAudio->Tick( Delta, nullptr );
	}

	// Render the editor.
	{
		profile_zone( EProfilerGroup::General, RenderTime )

		CCanvas* Canvas	= GRender->Lock();
		{
			// Render page.
			if( Active )
			{
				gfx::ScopedRenderingZone srz( TEXT( "Editor Page" ) );
				profile_zone( EProfilerGroup::General, RenderPage );
				Active->RenderPageContent( Canvas );
			}

			// Render editor GUI.
			{
				profile_zone( EProfilerGroup::General, RenderGUI );
				gfx::ScopedRenderingZone srz( TEXT( "Editor GUI" ) );

				GUIRender->BeginPaint( Canvas );
				{
					GUIWindow->WidgetProc( WPE_Paint, GUIRender );
				}
				GUIRender->EndPaint();
			}

			// update profiler
			{
				profile_zone( EProfilerGroup::General, RenderChart );
				m_engineChart->render( Canvas );
			}
		}
		GRender->Unlock();

		// Fooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
		{
			static Float lastUpdateTime = 0.f;
			lastUpdateTime += Delta;

			if( lastUpdateTime > 2.f )
			{
				res::ResourceManager::update();
				lastUpdateTime = 0.f;
			}
		}
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/