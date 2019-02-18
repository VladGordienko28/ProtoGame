//-----------------------------------------------------------------------------
//	Angle.cpp: sinus i cosinus - lactose
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Math.h"

namespace flu
{
namespace math
{
	typedef Float (*TrigonometryFunc)( Float );

	template<TrigonometryFunc FUNC, SizeT SIZE> class AutoTrigonometryTable final
	{
	public:
		AutoTrigonometryTable()
		{
			for( SizeT i = 0; i < SIZE; ++i )
			{
				m_table[i] = FUNC( 2.f * math::PI * i / Float(SIZE) );
			}
		}

		~AutoTrigonometryTable() = default;

		inline Float get( Int32 i ) const
		{
			return m_table[i];
		}

	private:
		Float m_table[SIZE];

		AutoTrigonometryTable( AutoTrigonometryTable<FUNC, SIZE>& ) = delete;
		AutoTrigonometryTable( AutoTrigonometryTable<FUNC, SIZE>&& ) = delete;
	};

	Float Angle::getCos() const
	{
		static AutoTrigonometryTable<math::cos, TABLE_SIZE> cosTable;
		return cosTable.get( ( value >> TABLE_BITS_BIAS ) & TABLE_MASK );
	}

	Float Angle::getSin() const
	{
		static AutoTrigonometryTable<math::sin, TABLE_SIZE> sinTable;
		return sinTable.get( ( value >> TABLE_BITS_BIAS ) & TABLE_MASK );
	}
}
}