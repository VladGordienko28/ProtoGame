/*=============================================================================
    FrHello.cpp: Hi dear user...
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WHelloPage implementation.
-----------------------------------------------------------------------------*/

//
// Page constructor.
//
WHelloPage::WHelloPage( WContainer* InOwner, WWindow* InRoot )
	:	WEditorPage( InOwner, InRoot )
{
	// Initialize own variables.
	PageType				= PAGE_Hello;
	Caption					= L"Start Page";
	TabWidth				= 96;
	Color					= PAGE_COLOR_HELLO;
		
	// Allocate project's links.	
	NewLink					= new WLinkLabel( this, Root );
	NewLink->Caption		= L"Create a new project...";
	NewLink->EventClick		= WIDGET_EVENT(WHelloPage::LinkNewClick);
	NewLink->SetSize( WWindow::Font1->TextWidth(*NewLink->Caption), WWindow::Font1->Height );

	OpenLink				= new WLinkLabel( this, Root );
	OpenLink->Caption		= L"Open an existing project...";
	OpenLink->EventClick	= WIDGET_EVENT(WHelloPage::LinkOpenClick);
	OpenLink->SetSize( WWindow::Font1->TextWidth(*OpenLink->Caption), WWindow::Font1->Height );

	// Links to recent projects.
	for( Int32 i=0; i<arraySize(Recent); i++ )
		RecentFiles[i]	= GEditor->Config->ReadString( L"Editor", L"Recent", *String::Format(L"Recent[%i]", i), L"" );

	// Allocate links.
	mem::zero( Recent, sizeof(Recent) );
	for( Int32 i=0; i<arraySize(Recent); i++ )
	{
		if( !RecentFiles[i] )
			break;

		Recent[i]				= new WLinkLabel( this, Root );
		Recent[i]->Caption		= GetFileName(RecentFiles[i]) + L".fluproj";
		Recent[i]->Tooltip		= RecentFiles[i];
		Recent[i]->EventClick	= WIDGET_EVENT(WHelloPage::LinkRecentClick);
		Recent[i]->SetSize( WWindow::Font1->TextWidth(*Recent[i]->Caption), WWindow::Font1->Height );
	}
}


//
// Draw hello page.
//
void WHelloPage::OnPaint( CGUIRenderBase* Render )
{
	TPoint Base = ClientToWindow(TPoint::Zero);
	Render->SetClipArea( Base, Size );

	// Draw backdrop.
	Render->DrawRegion
	(
		Base,
		Size,
		TColor( 0x33, 0x33, 0x33, 0xff ),
		TColor( 0x33, 0x33, 0x33, 0xff ),
		BPAT_Solid
	);
	Render->DrawRegion
	( 
		Base, 
		Size,
		TColor( 0x3a, 0x3a, 0x3a, 0xff ), 
		TColor( 0x3a, 0x3a, 0x3a, 0xff ), 
		BPAT_Diagonal 
	);

	// Bottom panel.
	Render->DrawRegion
	(
		TPoint( Base.X, Base.Y+Size.Height-45 ),
		TSize( Size.Width, 45 ),
		TColor( 0x20, 0x20, 0x20, 0xff ),
		TColor( 0x20, 0x20, 0x20, 0xff ),
		BPAT_Solid
	);
	Render->DrawText
	(
		TPoint( Base.X+(Size.Width-Root->Font1->TextWidth(FLU_COPYRIGHT))/2 ,Base.Y+Size.Height-30 ),
		FLU_COPYRIGHT,
		GUI_COLOR_TEXT,
		Root->Font1
	);

	// Top panel.
	Render->DrawRegion
	(
		Base,
		TSize( Size.Width, 90 ),
		TColor( 0x22, 0x22, 0x22, 0xff ),
		TColor( 0x22, 0x22, 0x22, 0xff ),
		BPAT_Solid
	);
	Render->DrawPicture
	(
		TPoint( Base.X+(Size.Width-256)/2, Base.Y+20 ),
		TSize( 256, 64 ),
		TPoint( 0, 192 ),
		TSize( 256, 64 ),
		Root->Icons
	);

	// Recent projects list.
	Render->DrawText
	(
		TPoint( Base.X+Max(Size.Width-200, 240), Base.Y+150 ),
		L"Recent projects",
		GUI_COLOR_TEXT,
		Root->Font1
	);
	for( Int32 i=0; i<arraySize(Recent); i++ )
		if( Recent[i] )
			Recent[i]->SetLocation
			(
				Max(Size.Width-200, 240),
				180 + i*25
			);

	// Projects list.
	Render->DrawText
	(
		TPoint( Base.X+50, Base.Y+150 ),
		L"Projects",
		GUI_COLOR_TEXT,
		Root->Font1
	);
	NewLink->SetLocation
	(
		50,
		180
	);
	OpenLink->SetLocation
	(
		50,
		205
	);
}


//
// Create new project clicked.
//
void WHelloPage::LinkNewClick( WWidget* Sender )
{
	GEditor->NewProject();
}


//
// Open new project clicked.
//
void WHelloPage::LinkOpenClick( WWidget* Sender )
{
	GEditor->OpenProject();
}


//
// Open an recent project.
//
void WHelloPage::LinkRecentClick( WWidget* Sender )
{
	String FileName;
	for( Int32 i=0; i<arraySize(Recent); i++ )
		if( Sender == Recent[i] )
		{
			FileName	= RecentFiles[i];
			break;
		}

	if( !GPlat->FileExists(FileName) )
	{
		Root->ShowMessage
		(
			String::Format( L"Unable to load project '%s'. File not found.", *FileName ),
			L"Error",
			true
		);
		return;
	}

	// Unload last project.
	if( GProject && !GEditor->CloseProject() )
		return;

	// Load it.
	GEditor->OpenProjectFrom(FileName);
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/