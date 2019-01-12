/*=============================================================================
    FrHello.h: Hello page.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WHelloPage.
-----------------------------------------------------------------------------*/

//
// Hello editor page.
//
class WHelloPage: public WEditorPage
{
public:
	// WHelloPage interface.
	WHelloPage( WContainer* InOwner, WWindow* InRoot );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );

public:
	// Variables.
	WLinkLabel*		NewLink;
	WLinkLabel*		OpenLink;
	WLinkLabel*		Recent[5];
	String			RecentFiles[5];

	// Controls notifications.
	void LinkNewClick( WWidget* Sender );
	void LinkOpenClick( WWidget* Sender );
	void LinkRecentClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/