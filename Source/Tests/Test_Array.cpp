//-----------------------------------------------------------------------------
//	Test_Array.cpp: Dynamic array tests
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Tests.h"

namespace flu
{
namespace tests
{
	void test_Array()
	{
		enter_unit( Array );

		check( 4 == 2 * 2 );
		check( 3 * 3 == 9 );

		//check( 'A' == 'B' );

		leave_unit;
	}
}
}