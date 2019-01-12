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
	// Get active page.
	WEditorPage* Active = (WEditorPage*)EditorPages->GetActivePage();
	
	// Tick active page.
	if( Active )
		Active->TickPage( Delta );

	// Render the editor.
	CCanvas* Canvas	= GRender->Lock();
	{
		// Render page.
		if( Active )
			Active->RenderPageContent( Canvas );

		// Render editor GUI.
		GUIRender->BeginPaint( Canvas );
		{
			GUIWindow->WidgetProc( WPE_Paint, GUIRender );
		}
		GUIRender->EndPaint();
	}
	GRender->Unlock();

	// Update audio.
	if( Active && Active->PageType == PAGE_Play )
		GAudio->Tick( Delta, ((WPlayPage*)Active)->PlayLevel );
	else
		GAudio->Tick( Delta, nullptr );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/