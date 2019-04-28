//-----------------------------------------------------------------------------
//	Test_Map.cpp: Dictionary tests
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Tests.h"

namespace flu
{
namespace tests
{
	void test_Map()
	{
		enter_unit( Map );

		// Map::Map
		{
			Map<Int32, Int32> emptyMap;
			Map<Int32, Int32> copyMap( emptyMap );

			check( emptyMap.size() == 0 );
			check( copyMap.size() == 0 );
			check( emptyMap.isEmpty() );
			check( copyMap.isEmpty() );
		}

		// Map::put
		{
			Map<Int32, String> myMap;
			
			myMap.put( 0, L"Alice" );
			check( myMap.size() == 1 );
			check( myMap.isEmpty() == false );

			myMap.put( 1, L"Bob" );
			check( myMap.size() == 2 );

			myMap.put( 2, L"Jake" );
			check( myMap.size() == 3 );

			myMap.put( 2, L"Vlad" );
			check( myMap.size() == 3 );
			check( myMap.isEmpty() == false );
		}

		// Map::empty
		{
			Map<String, String> capitals;
			check( capitals.isEmpty() == true );

			capitals.empty();
			check( capitals.isEmpty() == true );
			check( capitals.size() == 0 );

			capitals.put( L"USA", L"Washington" );
			capitals.put( L"Russia", L"Moscow" );
			capitals.put( L"Belarus", L"Minsk" );
			capitals.put( L"Italy", L"Rome" );
			capitals.put( L"GB", L"London" );

			check( capitals.isEmpty() == false );
			check( capitals.size() > 0 );
			capitals.empty();
			check( capitals.isEmpty() == true );
			check( capitals.size() == 0 );
		}

		// Map::hasKey & Map::hasValue
		{
			Map<String, math::Color> rainbow;
			rainbow.put( L"Red", math::colors::RED );
			rainbow.put( L"Orange", math::colors::ORANGE );
			rainbow.put( L"Yellow", math::colors::YELLOW );
			rainbow.put( L"Green", math::colors::GREEN );
			rainbow.put( L"Cyan", math::colors::CYAN );
			rainbow.put( L"Blue", math::colors::BLUE );
			rainbow.put( L"Violet", math::colors::BLUE_VIOLET );

			check( rainbow.hasKey( L"Red" ) == true );
			check( rainbow.hasKey( L"Blue" ) == true );
			check( rainbow.hasKey( L"Green" ) == true );

			check( rainbow.hasKey( L"Black" ) == false );
			check( rainbow.hasKey( L"Metal" ) == false );
			check( rainbow.hasKey( L"" ) == false );

			check( rainbow.hasValue( math::colors::RED ) == true );
			check( rainbow.hasValue( math::colors::YELLOW ) == true );

			check( rainbow.hasValue( math::colors::LIME ) == false );
			check( rainbow.hasValue( math::colors::WHITE ) == false );
		}

		// Map::get
		{
			Map<String, String> trafficLight;
			trafficLight.put( L"Red", L"Stop" );
			trafficLight.put( L"Yellow", L"Get Ready" );
			trafficLight.put( L"Green", L"Go" );

			check( trafficLight.get( L"Red" ) != nullptr );
			check( *trafficLight.get( L"Red" ) == L"Stop" );
			check( *trafficLight.get( L"Green" ) == L"Go" );
			check( trafficLight.get( L"Blue" ) == nullptr );
		}





#if 0



		/**
		 *	Put a new pair to the dictionary
		 *	Return false if existing pair were overrided and
		 *	return true if this is a new pair
		 */
		Bool put( const K& key, const V& value )
		{
			Int32 i = findPairIndex( key );

			if( i != -1 )
			{
				m_pairs[i].value = value;
				return false;
			}
			else
			{
				insertSorted( key, value );
				return true;
			}
		}

		/**
		 *	Return list of all keys
		 */
		Array<K> keys() const
		{
			Array<K> keys( m_pairs.size() );

			for( Int32 i = 0; i < m_pairs.size(); ++i )
			{
				keys[i] = m_pairs[i].key;
			}

			return static_cast<Array<K>&&>( keys );
		}

		/**
		 *	Return list of all values
		 */
		Array<V> values() const
		{
			Array<V> vals( m_pairs.size() );

			for( Int32 i = 0; i < m_pairs.size(); ++i )
			{
				vals[i] = m_pairs[i].value;
			}

			return static_cast<Array<V>&&>( vals );
		}

		/**
		 *	Remove a pair with specified key. Return false if 
		 *	key is not found
		 */
		Bool remove( const K& key )
		{
			Int32 i = findPairIndex( key );

			if( i != -1 )
			{
				m_pairs.removeShift( i );
				return true;
			}
			else
			{
				return false;
			}
		}

		Bool operator==( const Map<K, V>& other ) const
		{
			return m_pairs == other.m_pairs;
		}

		Bool operator!=( const Map<K, V>& other ) const
		{
			return m_pairs != other.m_pairs;
		}

		Map<K, V>& operator=( const Map<K, V>& other )
		{
			m_pairs = other.m_pairs;
			return *this;
		}

#endif




		leave_unit;
	}
}
}