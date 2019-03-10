/*=============================================================================
    FrPage.h: Abstract editor page.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Declaration.
-----------------------------------------------------------------------------*/

//
// An editor page type.
//
enum EPageType
{
	PAGE_None,			// Bad page type.
	PAGE_Hello,			// It's an intro page.
	PAGE_Texture,		// Bitmap edit page.
	PAGE_Level,			// Level edit page.
	PAGE_Animation,		// Animation edit page.
	PAGE_Script,		// Script edit page.
	PAGE_Play,			// Level testing page.
	PAGE_Skeleton,		// Skeleton management page.
	PAGE_MAX
};


//
// Pages colors.
//
#define PAGE_COLOR_HELLO		math::Color( 0x66, 0x66, 0x66, 0xff )
#define PAGE_COLOR_TEXTURE		math::Color( 0x00, 0x00, 0xcc, 0xff )
#define PAGE_COLOR_LEVEL		math::Color( 0xcc, 0x80, 0x00, 0xff )
#define PAGE_COLOR_ANIMATION	math::Color( 0xcc, 0xcc, 0x00, 0xff )
#define PAGE_COLOR_SCRIPT		math::Color( 0x00, 0xcc, 0x00, 0xff )
#define PAGE_COLOR_PLAY			math::Color( 0x00, 0xcc, 0xcc, 0xff )
#define PAGE_COLOR_SKELETON		math::Color( 0xcc, 0x00, 0x80, 0x00 )


/*-----------------------------------------------------------------------------
    WEditorPage.
-----------------------------------------------------------------------------*/

//
// An abstract editor page.
//
class WEditorPage: public WTabPage
{
public:
	// Variables.
	EPageType		PageType;

	// Constructors.
	WEditorPage( WContainer* InOwner, WWindow* InRoot )
		:	WTabPage( InOwner, InRoot ),
			PageType( PAGE_None )
	{}

	// WEditorPage interface.
	virtual void RenderPageContent( CCanvas* Canvas ){}
	virtual void TickPage( Float Delta ){}
	virtual void Undo(){}
	virtual void Redo(){}
	virtual FResource* GetResource(){ return nullptr; }
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/