/*=============================================================================
    FrEdMenu.h: Editor main menu bar.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WEditorMainMenu.
-----------------------------------------------------------------------------*/

//
// Main menu bar.
//
class WEditorMainMenu: public WMainMenu
{
public:
	// Sub menu.
	WMenu*		FileSubmenu;
	WMenu*		EditSubmenu;
	WMenu*		ProjectSubmenu;
	WMenu*		ResourceSubmenu;
	WMenu*		BuildSubmenu;
	WMenu*		HelpSubmenu;

	// WEditorMainMenu interface.
	WEditorMainMenu( WContainer* InOwner, WWindow* InRoot );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );

	// Menu notifications.
	void NewProjectClick( WWidget* Sender );
	void OpenProjectClick( WWidget* Sender );
	void SaveProjectClick( WWidget* Sender );
	void SaveProjectAsClick( WWidget* Sender );
	void CloseProjectClick( WWidget* Sender );
	void ExitClick( WWidget* Sender );
	void UndoClick( WWidget* Sender );
	void RedoClick( WWidget* Sender );
	void CutClick( WWidget* Sender );
	void CopyClick( WWidget* Sender );
	void PasteClick( WWidget* Sender );
	void DeleteClick( WWidget* Sender );
	void SelectAllClick( WWidget* Sender );
	void PreferencesClick( WWidget* Sender );
	void AddLevelClick( WWidget* Sender );
	void CompileScriptsClick( WWidget* Sender );
	void ProjectPropertiesClick( WWidget* Sender );
	void ImportResourceClick( WWidget* Sender );
	void BuildGameClick( WWidget* Sender );
	void RebuildGameClick( WWidget* Sender );
	void HelpClick( WWidget* Sender );
	void AboutClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/