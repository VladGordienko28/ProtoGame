/*=============================================================================
	FrRename.h: Resource rename dialog.
	Created by Vlad Gordienko, May. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	WRenameDialog.
-----------------------------------------------------------------------------*/

//
// A resource rename dialog.
//
class WRenameDialog: public WForm, public CRefsHolder
{
public:
	// WRenameDialog interface.
	WRenameDialog::WRenameDialog( WWindow* InRoot )
		:	WForm( InRoot, InRoot ),
			Resource( nullptr ),
			OldModal( nullptr )
	{
		// Initialize the form.
		Caption			= L"Rename Dialog";
		bCanClose		= true;
		bSizeableH		= false;
		bSizeableW		= false;
		SetSize( 240, 123 );

		TopPanel			= new WPanel( this, Root );
		TopPanel->SetLocation( 8, 28 );
		TopPanel->SetSize( 224, 57 );		

		NameLabel			= new WLabel( TopPanel, Root );
		NameLabel->Caption	= L"Name:";
		NameLabel->SetLocation( 8, 8 );

		NameEdit			= new WEdit( TopPanel, Root );
		NameEdit->EditType	= EDIT_String;
		NameEdit->Location	= TPoint( 56, 7 );
		NameEdit->SetSize( 160, 18 );

		GroupLabel			= new WLabel( TopPanel, Root );
		GroupLabel->Caption	= L"Group:";
		GroupLabel->SetLocation( 8, 32 );

		GroupEdit			= new WEdit( TopPanel, Root );
		GroupEdit->EditType	= EDIT_String;
		GroupEdit->Location	= TPoint( 56, 31 );
		GroupEdit->SetSize( 160, 18 );

		OkButton				= new WButton( this, Root );
		OkButton->Caption		= L"Ok";
		OkButton->EventClick	= WIDGET_EVENT(WRenameDialog::ButtonOkClick);
		OkButton->SetLocation( 38, 90 );
		OkButton->SetSize( 64, 25 );

		CancelButton				= new WButton( this, Root );
		CancelButton->Caption		= L"Cancel";
		CancelButton->EventClick	= WIDGET_EVENT(WRenameDialog::ButtonCancelClick);
		CancelButton->SetLocation( 138, 90 );
		CancelButton->SetSize( 64, 25 );

		Hide();
	}
	void WRenameDialog::SetResource( FResource* InResource )
	{
		assert(InResource);
		Resource	= InResource;

		NameEdit->Text	= Resource->GetName();
		GroupEdit->Text	= Resource->Group;

		// Turn off group for level.
		GroupEdit->bEnabled	=
		GroupLabel->bEnabled	= !Resource->IsA(FLevel::MetaClass);

		Show( Root->Size.Width/3, Root->Size.Height/3 );
	}

	// WForm interface.
	void WRenameDialog::Show( Int32 X, Int32 Y ) override
	{
		WForm::Show( X, Y );
		OldModal	= Root->Modal;
		Root->Modal	= this;
	}
	void WRenameDialog::OnClose() override
	{
		WForm::OnClose();
		Hide();
	}
	void WRenameDialog::Hide() override
	{
		WForm::Hide();
		Root->Modal		= OldModal;
	}

	// CRefsHolder interface.
	void WRenameDialog::CountRefs( CSerializer& S )
	{
		Serialize( S, Resource );
		if( !Resource )
			Hide();
	}

private:
	// Internal.
	FResource*			Resource;
	WLabel*				NameLabel;
	WLabel*				GroupLabel;
	WEdit*				NameEdit;
	WEdit*				GroupEdit;
	WPanel*				TopPanel;
	WButton*			OkButton;
	WButton*			CancelButton;
	WForm*				OldModal;

	// Notifications.
	void WRenameDialog::ButtonOkClick( WWidget* Sender )
	{
		if( NameEdit->Text.Len() == 0 )
		{
			Root->ShowMessage( L"Please specify new name of resource", L"Rename Dialog", true );
			return;
		}
		if( GObjectDatabase->FindObject( NameEdit->Text ) )
		{
			Root->ShowMessage
			(
				String::Format( L"Object \"%s\" already exists", *NameEdit->Text ),
				L"Rename Dialog",
				true
			);
			return;
		}

		// Change resource group.
		Resource->Group	= GroupEdit->Text;

		// Object renaming is tricky a little, since all
		// objects are hashed in global table.
		GObjectDatabase->RenameObject( Resource, NameEdit->Text );
		Hide();
	}
	void WRenameDialog::ButtonCancelClick( WWidget* Sender )
	{
		Hide();
	}
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/