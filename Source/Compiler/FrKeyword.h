/*=============================================================================
    FrKeyword.h: List of fluscript keywords.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

// It's messy a little. Store source macro.
// Its here since some keywords are C++ macro also.
#pragma push_macro("info")
#pragma push_macro("debug")
#pragma push_macro("warn")
#pragma push_macro("error")
#pragma push_macro("assert")
#undef info
#undef debug
#undef warn
#undef error
#undef assert

// List of keywords.
KEYWORD(null)
KEYWORD(false)
KEYWORD(true)
KEYWORD(undefined)
KEYWORD(nowhere)
KEYWORD(family)
KEYWORD(script)
KEYWORD(private)
KEYWORD(public)
KEYWORD(const)
KEYWORD(enum)
KEYWORD(byte)
KEYWORD(bool)
KEYWORD(integer)
KEYWORD(float)
KEYWORD(angle)
KEYWORD(color)
KEYWORD(string)
KEYWORD(vector)
KEYWORD(aabb)
KEYWORD(entity)
KEYWORD(fn)
KEYWORD(event)
KEYWORD(thread)
KEYWORD(result)
KEYWORD(if)
KEYWORD(for)
KEYWORD(do)
KEYWORD(switch)
KEYWORD(while)
KEYWORD(break)
KEYWORD(return)
KEYWORD(continue)
KEYWORD(stop)
KEYWORD(goto)
KEYWORD(sleep)
KEYWORD(wait)
KEYWORD(this)
KEYWORD(info)
KEYWORD(debug)
KEYWORD(warn)
KEYWORD(error)
KEYWORD(assert)
KEYWORD(else)
KEYWORD(is)
KEYWORD(base)
KEYWORD(length)
KEYWORD(push)
KEYWORD(pop)
KEYWORD(remove)
KEYWORD(label)
KEYWORD(new)
KEYWORD(delete)
KEYWORD(default)
KEYWORD(case)
KEYWORD(unified)
KEYWORD(in)
KEYWORD(out)
KEYWORD(proto)
KEYWORD(foreach)
KEYWORD(interrupt)
KEYWORD(struct)
KEYWORD(delegate)
KEYWORD(static)
KEYWORD(shell)

// Restore source macro.
#pragma pop_macro("assert")
#pragma pop_macro("error")
#pragma pop_macro("warn")
#pragma pop_macro("debug")
#pragma pop_macro("info")

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/