/*=============================================================================
    FrConsole.h: A console for cheats and game debugging.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CConsole.
-----------------------------------------------------------------------------*/

//
// Console buttons.
//
#define	CON_EXEC_BUTTON			13		// <Enter>
#define	CON_TOGGLE_BUTTON		9		// <Tab>


//
// Console text colors.
//
enum ETextColor
{
	TCR_White,
	TCR_Gray,
	TCR_Red,
	TCR_Green,
	TCR_Blue,
	TCR_Magenta,
	TCR_Cyan,
	TCR_Yellow,
	TCR_MAX
};


//
// A game console.
//
class CConsole: public CRefsHolder
{
public:
	// Constants.
	enum{ MAX_CON_HISTORY = 16 };

	// Static console font.
	static TStaticFont*	Font;

	// CConsole interface.
	CConsole();
	~CConsole();
	void Render( CCanvas* Canvas );
	void CharType( Char C );
	Bool ShowToggle();
	void LogCallback( String Msg, ETextColor Color );
	void Clear();

	// Accessors.
	inline Bool IsActive() const
	{
		return bActive;
	}

	// CRefsHolder interface.
	void CountRefs( CSerializer& S );

private:
	// Console internal.
	struct
	{
		String		Text;
		ETextColor	Color;
	}					
						History[MAX_CON_HISTORY];
	Bool				bActive;
	Int32				HistTop;
	String				Command;

	void Accept();
	void AddToHistory( String S, ETextColor Color );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/