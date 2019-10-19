/*=============================================================================
    FrTask.cpp: WTaskDialog.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WTaskDialog implementation.
-----------------------------------------------------------------------------*/

//
// Task dialog constructor.
//
WTaskDialog::WTaskDialog( WWindow* InRoot )
	:	WForm( InRoot, InRoot ),
		bInProgress( false ),
		OldModal( nullptr )
{
	// Setup own variables.
	Caption					= L"Unknown task";
	bCanClose				= false;
	bSizeableH				= false;
	bSizeableW				= false;
	SetSize( 400, 90 );	

	// Label.
	Label					= new WLabel( this, Root );
	Label->Caption			= L"Current Op: ";
	Label->Location			= TPoint( 12, 31 );

	// ProgressBar.
	ProgressBar				= new WProgressBar( this, Root );
	ProgressBar->Location	= TPoint( 10, 50 );
	ProgressBar->Color		= math::colors::CORNFLOWER_BLUE/*COLOR_DodgerBlue*/;
	ProgressBar->SetSize( 370, 25 );	
	ProgressBar->SetValue( 50 );

	Hide();
}


//
// Task dialog destructor.
//
WTaskDialog::~WTaskDialog()
{
	assert(!InProgress());
}


//
// Begin a slow task!
//
void WTaskDialog::BeginTask( String TaskName )
{
	assert(!InProgress());

	// Show at center.
	Show( (Root->Size.Width-Size.Width)/2, (Root->Size.Height-Size.Height)/2 );

	// Make it modal.
	OldModal	= Root->Modal;
	Root->Modal	= this;

	// Set fields.
	Caption				= TaskName;
	Label->Caption		= L"";
	ProgressBar->SetValue(0);

	// Mark as in-progress.
	bInProgress	= true;
}


//
// End slow task.
//
void WTaskDialog::EndTask()
{
	assert(InProgress());

	// No modal anymore.
	Root->Modal	= OldModal;
	OldModal	= nullptr;

	// Hide dialog.
	Hide();

	// Unmark in-progress.
	bInProgress	= false;
}


//
// Hide the task dialog.
//
void WTaskDialog::Hide()
{
	WForm::Hide();
}


//
// Redraw dialog and entire gui elements.
// Its hack a little, to process rendering
// here, not in FrEdTic.cpp, but works well.
//
void WTaskDialog::RedrawAll()
{
	// Render the editor.
	GEditor->m_world->onBeginUpdate();
	{
		WEditorPage* Active = GEditor->GetActivePage();

		// Render page.
		if( Active )
			Active->RenderPageContent( GEditor->m_legacyRender->m_canvas.get() );

		// Render editor GUI.
		GEditor->GUIRender->BeginPaint( GEditor->m_world->drawContext() );
		{
			GEditor->GUIWindow->WidgetProc( WPE_Paint, GEditor->GUIRender );
		}
		GEditor->GUIRender->EndPaint( GEditor->m_world->drawContext() );
	}
	GEditor->m_world->onEndUpdate();
}


//
// Update task progress value.
//
void WTaskDialog::SetProgress( Int32 Numerator, Int32 Denominator )
{
	assert(bInProgress);
	assert(Denominator>0);
	ProgressBar->SetValue(Numerator*100/Denominator);
	RedrawAll();
}


//
// Update subtask status.
//
void WTaskDialog::UpdateDetails( String SubtaskName )
{
	assert(bInProgress);
	Label->Caption	= SubtaskName;
	RedrawAll();
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/