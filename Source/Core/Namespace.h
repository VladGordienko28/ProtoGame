//-----------------------------------------------------------------------------
//	Namespace.h: A names manager
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#if 0

namespace flu
{
	/**
	 *	An immutable name in the namespace
	 */
	class Name
	{
	public:
		Name()
			:	m_namespaceId( -1 ),
				m_nameId( -1 )
		{
		}

		Bool isValid() const
		{
			return m_nameId != -1 && m_namespaceId != -1;
		}

		Bool operator==( const Name& other ) const
		{
			return m_nameId == other.m_nameId && m_namespaceId == other.m_namespaceId;
		}

		Bool operator!=( const Name& other ) const
		{
			return m_nameId != other.m_nameId || m_namespaceId != other.m_namespaceId;
		}

	private:
		Int32 m_nameId;
		Int32 m_namespaceId;

		Name( Int32 id, Int32 namespaceId )
			:	m_nameId( id ),
				m_namespaceId( namespaceId )
		{
		}

		template<typename CHARTYPE, SizeT HASH_TABLE_SIZE> friend class Namespace; 
	};

	/**
	 *	A container of unique names
	 */
	template<typename CHARTYPE, SizeT HASH_TABLE_SIZE> class Namespace
	{
	public:
		Namespace();
		~Namespace();


		Name findOrAddName( const CHARTYPE* name );

		Bool removeName( const Name& name );
		

		Name findName( const CHARTYPE* name ) const;

		Bool hasName( const CHARTYPE* name ) const
		{
			return *findEntry( name ) != nullptr;
		}

		const CHARTYPE* resolveName( const Name& name ) const
		{
			assert( name.isValid() );
			assert( isBelongNamespace( name ) );

			return m_entries[name.m_nameId]->name;
		}

		Bool isBelongNamespace( const Name& name ) const
		{
			return name.m_namespaceId == m_namespaceId;
		}

	private:
		static_assert( isPowerOfTwo(HASH_TABLE_SIZE), "Namespace hash table size is not power of two" );
		static const SizeT HASH_TABLE_MASK = HASH_TABLE_SIZE - 1;

		struct NameEntry
		{
			NameEntry* next = nullptr;
			CHARTYPE name[0] = {};
		};

		Int32 m_namespaceId;
		Array<NameEntry*> m_entries;
		Array<Int32> m_available;
		NameEntry* m_hashTable[HASH_TABLE_SIZE];
 
		Namespace( Namespace<CHARTYPE, HASH_TABLE_SIZE>& ) = delete;
		Namespace( Namespace<CHARTYPE, HASH_TABLE_SIZE>&& ) = delete;
		Namespace& operator=( const Namespace<CHARTYPE, HASH_TABLE_SIZE>& ) = delete;

		NameEntry** findEntry( const CHARTYPE* name )
		{
			UInt32 hash = cstr::hash( name );
			NameEntry** entryPtr = &m_hashTable[hash & HASH_TABLE_MASK];

			while( *entryPtr && cstr::compare( *entryPtr->name, name ) != 0 )
			{
				entryPtr = &(*entryPtr)->next;
			}

			return entryPtr;
		}
	};
}

#endif