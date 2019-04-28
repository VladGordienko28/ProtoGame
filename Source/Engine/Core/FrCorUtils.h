/*=============================================================================
	FrCorUtils.h: A various core utils.
	Created by Vlad Gordienko, Dec. 2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
	TDelegate.
-----------------------------------------------------------------------------*/

//
// A FluScript delegate.
//
struct TDelegate
{
public:
	// Variables.
	Int32		iMethod;
	FScript*	Script;
	FEntity*	Context;

	// TDelegate interface.
	TDelegate()
		:	iMethod(-1), Script(nullptr), Context(nullptr)
	{}
	TDelegate( Int32 InMethod, FScript* InScript, FEntity* InContext )
		:	iMethod(InMethod),
			Script(InScript),
			Context(InContext)
	{}

	// Operators.
	operator Bool() const
	{
		return iMethod != -1 && Script && Context;
	}

	// Friends.
	friend void Serialize( CSerializer& S, TDelegate& V );
};


/*-----------------------------------------------------------------------------
	TUrl.
-----------------------------------------------------------------------------*/

//
// An uniform resource locator.
//
class TUrl
{
public:
	// Variables.
	String	Protocol;
	String	Login;
	String	Password;
	String	Host;
	Int32	Port;
	String	Path;
	String	Fragment;
	Map<String, String> Query;

	// TUrl interface.
	TUrl();
	TUrl( String InUrl );
	TUrl( const TUrl& BaseUrl, String RelativeUrl );
	String ToString() const;
	Bool operator==( const TUrl& Other ) const;
	Bool operator!=( const TUrl& Other ) const;
};


/*-----------------------------------------------------------------------------
	Functions.
-----------------------------------------------------------------------------*/

//
// Hash functions.
//
extern UInt32 MurmurHash( const UInt8* Data, SizeT Size );


/*-----------------------------------------------------------------------------
	IProgressIndicator.
-----------------------------------------------------------------------------*/

//
// An abstract progress indicator.
//
class IProgressIndicator
{
public:
	// IProgressIndicator interface.
	virtual void BeginTask( String TaskName ) = 0;
	virtual void EndTask() = 0;
	virtual void UpdateDetails( String Details ) = 0;
	virtual void SetProgress( Int32 Numerator, Int32 Denominator ) = 0;

	struct THolder
	{
	public:
		IProgressIndicator* Indicator;

		THolder( IProgressIndicator* InIndicator, String TaskName )
			:	Indicator( InIndicator )
		{
			if( Indicator )
				Indicator->BeginTask( TaskName );
		}
		~THolder()
		{
			if( Indicator )
				Indicator->EndTask();
		}
		void UpdateDetails( String Details )
		{
			if( Indicator )
				Indicator->UpdateDetails(Details);
		}
		void SetProgress( Int32 Numerator, Int32 Denominator )
		{
			if( Indicator )
				Indicator->SetProgress( Numerator, Denominator );
		}
	};
};


/*-----------------------------------------------------------------------------
	CCmdLineParser.
-----------------------------------------------------------------------------*/

//
// A command line parser.
//
class CCmdLineParser
{
public:
	// Parse the first word of the line.
	// This is should be global command of the line.
	static String ParseCommand( String Line, Int32 iArgNum=0 );

	// Parse a flag-option from the line. 
	// This routine recognize word/char option such as:
	// -v or --verbose.
	static Bool ParseOption( String Line, String Option );

	// Parse a value param in form:
	// Name="Bot0" or Mode=1
	static Bool ParseBoolParam( String Line, String Name, Bool Default=false );
	static Int32 ParseIntParam( String Line, String Name, Int32 Default=0 );
	static Float ParseFloatParam( String Line, String Name, Float Default=0.f );
	static String ParseStringParam( String Line, String Name, String Default=L"" );
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/