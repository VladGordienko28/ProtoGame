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

		m_world->onBeginUpdate();
		{

			// Render page.
			if( Active )
			{
				gfx::ScopedRenderingZone srz( TXT( "Editor Page" ) );
				profile_zone( EProfilerGroup::General, RenderPage );
				profile_gpu_zone( Page );

				Active->RenderPageContent( m_legacyRender->m_canvas.get() );
			}

			// Render editor GUI.
			{
				profile_zone( EProfilerGroup::General, RenderGUI );
				gfx::ScopedRenderingZone srz( TXT( "Editor GUI" ) );
				profile_gpu_zone( GUI );

				GUIRender->BeginPaint( m_world->drawContext() );
				{
					GUIWindow->WidgetProc( WPE_Paint, GUIRender );
				}
				GUIRender->EndPaint( m_world->drawContext() );
			}
		}
		m_world->onEndUpdate(GEditor->m_legacyRender->m_canvas.get());

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