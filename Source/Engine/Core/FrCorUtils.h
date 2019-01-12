/*=============================================================================
	FrCorUtils.h: A various core utils.
	Created by Vlad Gordienko, Dec. 2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
	TTimeOfDay.
-----------------------------------------------------------------------------*/

//
// A day period.
//
enum EDayPeriod
{
	PERIOD_Am,
	PERIOD_Pm
};


//
// Represents the time of the day.
//
class TTimeOfDay
{
public:
	// TTimeOfDay interface.
	TTimeOfDay()
		:	DaySeconds(0.f)
	{}
	TTimeOfDay( Integer InHour, Integer InMinute=0, Integer InSecond=0 )
		:	DaySeconds(((InHour*60.f)+InMinute)*60.f+InSecond*1.f)
	{}
	void Tick( Float DeltaSecs )
	{
		DaySeconds	= FMod( DaySeconds+DeltaSecs, 86400.f );
	}
	EDayPeriod GetPeriod() const
	{
		return DaySeconds < 43200.f ? PERIOD_Am : PERIOD_Pm;
	}
	Float ToPercent() const
	{
		return DaySeconds / 86400.f;
	}
	void FromPercent( Float p )
	{
		DaySeconds	= p * 86400.f;
	}
	String ToString() const
	{
		Integer Hour	= Integer(DaySeconds)/60/60 % 12;
		Integer Minute	= (Integer(DaySeconds) % 3600)/60;
		Integer Second	= Integer(DaySeconds) % 60;
		if( Hour == 0 && GetPeriod()==PERIOD_Pm )
			Hour	= 12;
		return String::Format( L"%02d:%02d:%02d %s", Hour, Minute, Second, GetPeriod()==PERIOD_Am?L"Am" : L"Pm" );
	}

private:
	// Internal.
	Float	DaySeconds;
};


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
	Integer		iMethod;
	FScript*	Script;
	FEntity*	Context;

	// TDelegate interface.
	TDelegate()
		:	iMethod(-1), Script(nullptr), Context(nullptr)
	{}
	TDelegate( Integer InMethod, FScript* InScript, FEntity* InContext )
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
	Integer	Port;
	String	Path;
	String	Fragment;
	TMap<String, String> Query;

	// TUrl interface.
	TUrl();
	TUrl( String InUrl );
	TUrl( const TUrl& BaseUrl, String RelativeUrl );
	String ToString() const;
	Bool operator==( const TUrl& Other ) const;
	Bool operator!=( const TUrl& Other ) const;
	friend void Serialize( CSerializer& S, TUrl& V );
};


/*-----------------------------------------------------------------------------
	Functions.
-----------------------------------------------------------------------------*/

//
// Hash functions.
//
extern DWord MurmurHash( const Byte* Data, SizeT Size );


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
	virtual void SetProgress( Integer Numerator, Integer Denominator ) = 0;

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
		void SetProgress( Integer Numerator, Integer Denominator )
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
	static String ParseCommand( String Line, Integer iArgNum=0 );

	// Parse a flag-option from the line. 
	// This routine recognize word/char option such as:
	// -v or --verbose.
	static Bool ParseOption( String Line, String Option );

	// Parse a value param in form:
	// Name="Bot0" or Mode=1
	static Bool ParseBoolParam( String Line, String Name, Bool Default=false );
	static Integer ParseIntParam( String Line, String Name, Integer Default=0 );
	static Float ParseFloatParam( String Line, String Name, Float Default=0.f );
	static String ParseStringParam( String Line, String Name, String Default=L"" );
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/