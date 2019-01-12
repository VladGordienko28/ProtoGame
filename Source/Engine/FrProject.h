/*=============================================================================
    FrProject.h: Complete game project.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

//
// An extension of project files.
//
#define PROJ_FILE_EXT		L".flg"
#define RES_FILE_EXT		L".flr"


/*-----------------------------------------------------------------------------
    CProject.
-----------------------------------------------------------------------------*/

//
// A complete game/project.
//
class CProject: public CObjectDatabase
{
public:
	// Variables.
	String				ProjName;
	String				FileName;
	FProjectInfo*		Info;
	CBlockManager*		BlockMan;

	// CProject interface.
	CProject();		
	~CProject();
	void SerializeProject( CSerializer& S );

	// Level management functions.
	FLevel* DuplicateLevel( FLevel* Source ); 
};


//
// Global accessible instance of project.
//
extern CProject*	GProject;


/*-----------------------------------------------------------------------------
    FProjectInfo.
-----------------------------------------------------------------------------*/

//
// An application window type.
//
enum EAppWindowType
{
	WT_Sizeable,
	WT_Single,
	WT_FullScreen
};


//
// An information resource about entire project
// which it is resided in.
//
class FProjectInfo: public FResource
{
REGISTER_CLASS_H(FProjectInfo);
public:
	// Variables.
	String				GameName;
	String				Author;
	Bool				bNoPause;

	// Window relative.
	Bool				bQuitByEsc;
	EAppWindowType		WindowType;
	Integer				DefaultWidth;
	Integer				DefaultHeight;

	// FProjectInfo interface.
	FProjectInfo();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/