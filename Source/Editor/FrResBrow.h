/*=============================================================================
	FrResBrowser.h: A resource browser panel.
	Created by Vlad Gordienko, Jul. 2016.
	Redesigned and reimplemented by Vlad Gordienko, Apr. 2018.
=============================================================================*/

#include "FrMusPlay.h"
#include "FrFontView.h"
#include "FrRename.h"

/*-----------------------------------------------------------------------------
	Forward declaration.
-----------------------------------------------------------------------------*/

class WAssetsPage;
class WResourcePane;
class WResourceBrowser;
class WScriptsPage;
class WLevelsPage;
class WLevelsList;
class WScriptsView;

/*-----------------------------------------------------------------------------
	WResourceList.
-----------------------------------------------------------------------------*/

//
// A List of resources.
//
class WResourceList: public WListBox
{
public:
	// WResourceList interface.
	WResourceList( WAssetsPage* InPage, WWindow* InRoot );
	~WResourceList();
	void Refresh();

	// WWidget events.
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y ) override;

	// WList interface.
	void OnDoubleClick() override;
	void OnChange() override;

private:
	// Internal.
	WAssetsPage*	Page;
	Bool			bReadyForDrag;
};


/*-----------------------------------------------------------------------------
	WResourcePane.
-----------------------------------------------------------------------------*/

//
// A Pane with resource tiles.
//
class WResourcePane: public WPanel, public CRefsHolder
{
public:
	// Hardcoded icons size.
	enum{ RES_ICON_SIZE = 64 };

	// WResourcePane interface.
	WResourcePane( WAssetsPage* InPage, WWindow* InRoot );
	~WResourcePane();
	void Refresh();

	// CRefHolder interface.
	void CountRefs( CSerializer& S ) override;

	// WWidget events.
	void OnPaint( CGUIRenderBase* Render ) override;
	void OnMouseScroll( Integer Delta ) override;
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y ) override;
	void OnDblClick( EMouseButton Button, Integer X, Integer Y ) override;    
	void OnResize() override;

private:
	// A resource icon.
	struct TIcon
	{
		FResource*	Resource;
		FTexture*	Picture;
		const Char*	TypeName;
		TPoint		Position;
		TPoint		PicOffset;
		TSize		PicSize;
		TSize		Scale;
	};

	// Internal.
	WAssetsPage*	Page;
	WSlider*		ScrollBar;
	TArray<TIcon>	Icons;
	Bool			bReadyForDrag;

	// Notifications.
	void ScrollChange( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
	WAssetsPage.
-----------------------------------------------------------------------------*/

//
// An assets page.
//
class WAssetsPage: public WTabPage, public CRefsHolder
{
public:
	// WAssetsPage interface.
	WAssetsPage( WResourceBrowser* InBrowser, WContainer* InOwner, WWindow* InRoot );
	~WAssetsPage();
	void Refresh();
	void ShowResourceMenu( FResource* Resource );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render ) override;

private:
	// Widgets.
	WPictureButton*		ImportButton;
	WPictureButton*		CreateButton;
	WPictureButton*		ClassButton;
	WPictureButton*		RemoveButton;
	WPictureButton*		ListViewButton;
	WPictureButton*		TileViewButton;
	WEdit*				NameFilter;
	WResourceList*		ResourceList;
	WResourcePane*		ResourcePane;

	// Internal.
	WResourceBrowser*	Browser;
	WPopupMenu*			NewPopup;
	WPopupMenu*			ResourcePopup;
	WPopupMenu*			ClassPopup;	

	// Resource relative.
	CClass*				ClassFilter;

	// Buttons callbacks.
	void ButtonListViewClick( WWidget* Sender );
	void ButtonTileViewClick( WWidget* Sender );
	void ButtonImportClick( WWidget* Sender );
	void ButtonCreateClick( WWidget* Sender );
	void ButtonClassClick( WWidget* Sender );
	void ButtonRemoveClick( WWidget* Sender );

	// New Resource PopUp.
	void PopNewEffectClick( WWidget* Sender );
	void PopNewAnimationClick( WWidget* Sender );
	void PopNewSkeletonClick( WWidget* Sender );
	void PopNewMaterialClick( WWidget* Sender );
	void PopNewScriptClick( WWidget* Sender );

	// Resource Edit PopUp;
	void PopEditClick( WWidget* Sender );
	void PopRenameClick( WWidget* Sender );
	void PopRemoveClick( WWidget* Sender );

	// Other callbacks.
	void EditNameChange( WWidget* Sender );
	void PopClassAllClick( WWidget* Sender );
	void PopClassClick( WWidget* Sender );

	// Friends.
	friend WResourceBrowser;
	friend WResourceList;
	friend WResourcePane;
	friend WLevelsList;
	friend WScriptsPage;

private:
	// All assets builders.
	WForm*		DemoEffectBuilder;
	WForm*		AnimationBuilder;
	WForm*		SkeletonBuilder;
	WForm*		MaterialBuilder;
	WForm*		ScriptBuilder;

	// Special dialogs.
	WForm*				ImportDialog;
	WMusicPlayer*		MusicPlayer;
	WFontViewDialog*	FontViewDialog;
	WRenameDialog*		RenameDialog;
};


/*-----------------------------------------------------------------------------
	WLevelsPage.
-----------------------------------------------------------------------------*/

//
// A levels page.
//
class WLevelsPage: public WTabPage, public CRefsHolder
{
public:
	// WScriptsPage interface.
	WLevelsPage( WResourceBrowser* InBrowser, WContainer* InOwner, WWindow* InRoot );
	~WLevelsPage();
	void Refresh();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render ) override;	

	// CRefHolder interface.
	void CountRefs( CSerializer& S ) override;

private:
	// Widgets.
	WPictureButton*		AddLevelButton;
	WPictureButton*		DeleteLevelButton;
	WPictureButton*		ProjectPropsButton;
	WListBox*			LevelsList;
	WForm*				LevelConstructor;

	// Internal.
	WResourceBrowser*	Browser;

	// Widgets notifications.
	void ButtonAddLevelClick( WWidget* Sender );
	void ButtonDeleteLevelClick( WWidget* Sender );
	void ButtonProjectPropsClick( WWidget* Sender );
	void ListLevelsChange( WWidget* Sender );
	void ListLevelsDblClick( WWidget* Sender );
	void MessageDeleteYesClick( WWidget* Sender );

	// Friends.
	friend WLevelsList;
	friend WEditorMainMenu;
};


/*-----------------------------------------------------------------------------
	WScriptsPage.
-----------------------------------------------------------------------------*/

//
// A scripts page.
//
class WScriptsPage: public WTabPage, public CRefsHolder
{
public:
	// WScriptsPage interface.
	WScriptsPage( WResourceBrowser* InBrowser, WContainer* InOwner, WWindow* InRoot );
	~WScriptsPage();
	void Refresh();

	// WTabPage interface.
	void OnOpen() override;

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render ) override;	

	// CRefHolder interface.
	void CountRefs( CSerializer& S ) override;

private:
	// Controls.
	WTreeView*			ScriptsView;
	WCheckBox*			EntityOnlyCheck;
	WPictureButton*		CreateButton;
	WPictureButton*		EditButton;
	WPictureButton*		CompileAllButton;
	WPictureButton*		RemoveButton;

	// Internal.
	WResourceBrowser*	Browser;

	// Widgets notifications.
	void ButtonCreateClick( WWidget* Sender );
	void ButtonEditClick( WWidget* Sender );
	void ButtonCompileAllClick( WWidget* Sender );
	void ButtonRemoveClick( WWidget* Sender );
	void CheckEntityOnlyChange( WWidget* Sender );
	void ViewDblClick( WWidget* Sender );
	void ViewSelectionChange( WWidget* Sender );

	// Friends.
	friend WResourceBrowser;
	friend WScriptsView;
};


/*-----------------------------------------------------------------------------
	WResourceBrowser.
-----------------------------------------------------------------------------*/

//
// A resource browser.
//
class WResourceBrowser: public WContainer, public CRefsHolder		
{
public:
	// Browser pages.
	WAssetsPage*	AssetsPage;
	WScriptsPage*	ScriptsPage;
	WLevelsPage*	LevelsPage;

	// WResourceBrowser interface.
	WResourceBrowser( WContainer* InOwner, WWindow* InRoot );
	~WResourceBrowser();
	void ActivateResource( FResource* Resource );
	void Refresh();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render ) override;

	// Accessors.
	inline FResource* GetSelected() const
	{
		return Selected;
	}

private:
	// Controls.
	WTabControl*	TopPanel;
	WPanel*			BottomPanel;	//!! packages placeholder.

	// Internal.
	FResource*		Selected;

	// Friends.
	friend WAssetsPage;
	friend WLevelsPage;
	friend WScriptsPage;
	friend WResourceList;
	friend WResourcePane;
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/