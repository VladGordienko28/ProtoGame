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
	profile_counter( EProfilerGroup::Memory, Allocated_Kb, mem::stats().totalAllocatedBytes / 1024.0 );

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
				profile_zone( EProfilerGroup::General, RenderPage );
				Active->RenderPageContent( Canvas );
			}

			// Render editor GUI.
			{
				profile_zone( EProfilerGroup::General, RenderGUI );

				GUIRender->BeginPaint( Canvas );
				{
					GUIWindow->WidgetProc( WPE_Paint, GUIRender );
				}
				GUIRender->EndPaint();
			}

			// update profiler
			{
				profile_zone( EProfilerGroup::General, RenderChart );
				m_engineChart.render( Canvas, WWindow::Font1 );
			}
		}
		GRender->Unlock();
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/