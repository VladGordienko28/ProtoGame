//-----------------------------------------------------------------------------
//	Array.cpp: Array internal magic
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

namespace flu 
{
namespace array
{
	/**
	  *	Figure out how many additional items should be allocated for
	  *	the array. This depends only on element size. Result is always a power
	  *	of two
	  */
	static inline constexpr Int32 arrayReservationSize( SizeT elementSize )
	{
		if( elementSize <= 1 )
			return 128;
		else if( elementSize <= 4 )
			return 64;
		else if( elementSize <= 8 )
			return 32;
		else 
			return 16;
	}


	void reallocate( void*& data, Int32& oldSize, Int32 newSize, SizeT elementSize )
	{
		const Int32 reservationSize = arrayReservationSize( elementSize );

		if( newSize == 0 )
		{
			// Get rid of data
			mem::free( data );
			data = nullptr;
			oldSize = 0;
		} 
		else if( data == nullptr )
		{
			// Allocate new data
			Int32 realSize = alignValue( newSize, reservationSize );
			data = mem::alloc( realSize * elementSize );
			oldSize = newSize;
		}
		else
		{
			// Reallocate array
			if( newSize > oldSize )
			{
				// Add new items
				Int32 realOldSize = alignValue( oldSize, reservationSize );
				if( newSize >= realOldSize )
				{
					// Need extra items
					Int32 realNewSize = alignValue( newSize, reservationSize );
					data = mem::realloc( data, realNewSize * elementSize );
					mem::zero
					( 
						reinterpret_cast<UInt8*>( data ) + oldSize * elementSize,
						( newSize - oldSize ) * elementSize
					);
				}
				else
				{
					// Memory enough, but need zero
					mem::zero
					( 
						reinterpret_cast<UInt8*>( data ) + oldSize * elementSize,
						( newSize - oldSize ) * elementSize
					);
				}

				oldSize = newSize;
			}
			else
			{
				// Remove some items
				Int32 realNewSize = alignValue( newSize, reservationSize );
				Int32 realOldSize = alignValue( oldSize, reservationSize );

				if( realOldSize != realNewSize )
					data = mem::realloc( data, realNewSize * elementSize );

				oldSize = newSize;
			}
		}
	}
} // namespace array
} // namespace flu