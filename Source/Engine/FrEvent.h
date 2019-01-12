/*=============================================================================
	FrEvent.h: List of fluscript events.
	Copyright Jan.2017 Vlad Gordienko.
	Improved by Vlad Gordienko Feb. 2018.
=============================================================================*/

//
// List of all events.
//
SCRIPT_EVENT( OnAnimEnd, END )
SCRIPT_EVENT( OnBeginPlay, END )
SCRIPT_EVENT( OnBeginTouch, ARG(other, FEntity*, END) )
SCRIPT_EVENT( OnCharType, ARG(c, String, END) )
SCRIPT_EVENT( OnCollide, ARG(other, FEntity*, ARG(hitSide, enum EHitSide, END)) )
SCRIPT_EVENT( OnDestroy, END )
SCRIPT_EVENT( OnEndPlay, END )
SCRIPT_EVENT( OnEndTouch, ARG(other, FEntity*, END) )
SCRIPT_EVENT( OnGetOrder, ARG(leader, FEntity*, ARG(order, String, END)) )
SCRIPT_EVENT( OnHearNoise, ARG(other, FEntity*, END) )
SCRIPT_EVENT( OnHide, END )
SCRIPT_EVENT( OnKeyDown, ARG(key, Integer, END) )
SCRIPT_EVENT( OnKeyUp, ARG(key, Integer, END) )
SCRIPT_EVENT( OnKeyPressed, ARG(key, Integer, END) )
SCRIPT_EVENT( OnLookAt, ARG(other, FEntity*, END) )
SCRIPT_EVENT( OnMirrorPass, ARG(mirror, FEntity*, END) )
SCRIPT_EVENT( OnPreTick, ARG(delta, Float, END) )
SCRIPT_EVENT( OnProcess, ARG(cmd, String, ARG(arg, String, END)) )
SCRIPT_EVENT( OnReceiveSignal, ARG(creator, FEntity*, ARG(entity, FEntity*, ARG(jackName, String, END))) )
SCRIPT_EVENT( OnRender, END )
SCRIPT_EVENT( OnShow, END )
SCRIPT_EVENT( OnStep, ARG(delta, Float, END) )
SCRIPT_EVENT( OnTick, ARG(delta, Float, END) )
SCRIPT_EVENT( OnWarpPass, ARG(warp, FEntity*, END) )
SCRIPT_EVENT( OnZoneChange, END )


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/