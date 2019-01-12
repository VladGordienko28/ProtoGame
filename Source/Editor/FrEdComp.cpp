/*=============================================================================
	FrEdComp.cpp: In-editor compiler integration.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    Editor functions.
-----------------------------------------------------------------------------*/

//
// Compile all scripts.
//
Bool CEditor::CompileAllScripts( IProgressIndicator* Indicator )
{
	// Don't compile if no project.
	if( !Project )
		return false;

	// Tell to user.
	info( L"Compiler: Compilation preparation" );

	IProgressIndicator::THolder Ind( Indicator, L"Compiling" );
	Ind.UpdateDetails(L"Prepare...");

	// Shutdown all play-pages, since in play-time all used script
	// objects are turns invalid. In many cases script
	// threads crash application. 
	{
		Bool bAgain	= true;
		while( bAgain )
		{
			bAgain	= false;
			for( Integer i=0; i<EditorPages->Pages.Num(); i++ )
			{
				WEditorPage* EdPage = (WEditorPage*)EditorPages->Pages[i];
				if( EdPage->PageType == PAGE_Play )
				{
					EdPage->Close( true );
					bAgain	= true;
					break;
				}
			}
		}
	}

	// Reset Undo/Redo for each level, since hardcoded
	// instance buffer.
	for( Integer i=0; i<EditorPages->Pages.Num(); i++ )
	{
		WEditorPage* EdPage = (WEditorPage*)EditorPages->Pages[i];
		if( EdPage->PageType == PAGE_Level )
		{
			WLevelPage* Page	= (WLevelPage*)EdPage;
			Page->Transactor->Reset();
		}
	}

	// Collect the script pages.
	TArray<WScriptPage*>	Pages;
	for( Integer i=0; i<EditorPages->Pages.Num(); i++ )
	{
		WEditorPage* EdPage = (WEditorPage*)EditorPages->Pages[i];
		
		if( EdPage->PageType == PAGE_Script )
			Pages.Push( (WScriptPage*)EdPage );
	}

	// Save text of each text.
	for( Integer i=0; i<Pages.Num(); i++ )
		Pages[i]->SaveScriptText( false );

	// Reset inspector since it refer CProperty.
	TArray<FObject*> tmp;
	Inspector->SetEditObjects( tmp );

	// Launch the compiler.
	Bool Result;
	TArray<String> Warns;
	Compiler::TError Err;

	Ind.UpdateDetails(L"Compiling...");

	if( Result = Compiler::CompileAllScripts(GProject, Warns, Err) )		
	{
		// Compilation successfully.
		for( Integer i=0; i<Pages.Num(); i++ )
		{
			Pages[i]->Output->Clear();

			for( Integer w=0; w<Warns.Num(); w++ )
				Pages[i]->Output->AddMessage( Warns[w], nullptr, -1, COLOR_Goldenrod );

			Pages[i]->Output->AddMessage( L"Compilation successfully", nullptr, -1, COLOR_Green );
		}
	}
	else
	{
		// Compilation failed.

		// Find and open the problem page.
		WScriptPage* Problem = nullptr;
		for( Integer i=0; i<Pages.Num(); i++ )
			if( Pages[i]->Script == Err.Script )
			{
				Problem	= Pages[i];
				break;
			}

		// If problem page not found - open it!
		if( !Problem )
			Problem	= (WScriptPage*)GEditor->OpenPageWith( Err.Script );

		// Highlight the error.
		Problem->HighlightError( Err.ErrorLine-1 );
		Pages.Push( Problem );

		// Forced open problem page.
		GEditor->EditorPages->ActivateTabPage(Problem);

		// Output messages.
		for( Integer i=0; i<Pages.Num(); i++ )
		{
			Pages[i]->Output->Clear();

			for( Integer w=0; w<Warns.Num(); w++ )
				Pages[i]->Output->AddMessage( Warns[w], nullptr, -1, COLOR_Goldenrod );

			Pages[i]->Output->AddMessage( L"Compilation aborted", Err.Script, Err.ErrorLine, TColor( 0xf0, 0x30, 0x30, 0xff ) );
			Pages[i]->Output->AddMessage( Err.Message, Err.Script, Err.ErrorLine, TColor( 0xf0, 0x30, 0x30, 0xff ) );
		}
	}

	// Notify.
	info( L"Compiler: Compilation finished." );
	return Result;
}


//
// Drop all project's scripts.
//
Bool CEditor::DropAllScripts()
{
	if( !Project )
		return false;

	// Shutdown all play-pages, since in play-time all used script
	// objects are turns invalid. In many cases script
	// threads crash application. 
	{
		Bool bAgain	= true;
		while( bAgain )
		{
			bAgain	= false;
			for( Integer i=0; i<EditorPages->Pages.Num(); i++ )
			{
				WEditorPage* EdPage = (WEditorPage*)EditorPages->Pages[i];
				if( EdPage->PageType == PAGE_Play )
				{
					EdPage->Close( true );
					bAgain	= true;
					break;
				}
			}
		}
	}

	// Drop it.
	Bool Result = Compiler::DropAllScripts( GProject );

	// Notify.
	info( L"Compiler: Scripts are were dropped." );
	return Result;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/