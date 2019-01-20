/*=============================================================================
    FrMap.h: An associative array template.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    TMap.
-----------------------------------------------------------------------------*/

//
// A well optimized dictionary.
//
template<class K, class V> class TMap
{
public:
	// Constructors.
	TMap()
		:	Entries()
	{}
	TMap( const TMap<K, V>& Other )
		:	Entries(Other.Entries)
	{}

	// Destructor.
	~TMap()
	{
		Entries.empty();
	}

	// Clear map.
	void Clear()
	{
		Entries.Empty();
	}

	// Return true, if map has this key.
	Bool ContainsKey( const K& Key ) const
	{
		return FindEntry(Key) != -1;
	}

	// Return true, if map has this value.
	Bool ContainsValue( const V& Value ) const
	{
		for( Int32 i=0; i<Entries.Num(); i++ )
			if( Entries[i].Value == Value )
				return true;
		return false;
	}

	// Get a value by key, if value not found return
	// nullptr.
	V* Get( const K& Key )
	{
		Int32 i = FindEntry(Key);
		return i != -1 ? &Entries[i].Value : nullptr;
	}
	const V* Get( const K& Key ) const
	{
		Int32 i = FindEntry(Key);
		return i != -1 ? &Entries[i].Value : nullptr;
	}

	// Return true, if map are empty.
	Bool IsEmpty() const
	{
		return Entries.Num() == 0;
	}

	// Add a new entry to map, if it found,
	// just override old.
	void Put( const K& Key, const V& Value )
	{
		Int32 i = FindEntry(Key);
		if( i != -1 )
			Entries[i].Value = Value;
		else
			Add( Key, Value );
	}

	// Return list of keys.
	Array<K> KeySet() const
	{
		TArray<K> Keys(Entries.Num());
		for( Int32 i=0; i<Entries.Num(); i++ )
			Keys[i] = Entries[i].Key;
		return Keys;
	}

	// Return list of values.
	Array<V> Values() const
	{
		TArray<V> Vals(Entries.Num());
		for( Int32 i=0; i<Entries.Num(); i++ )
			Vals[i] = Entries[i].Value;
		return Vals;
	}

	// Remove an entry by key. Return false, if
	// no key found.
	Bool Remove( const K& Key )
	{
		Int32 i = FindEntry(Key);
		if( i != -1 )
			Entries.RemoveShift(i);
		return i != -1;
	}

	// Return map size.
	Int32 Size() const
	{
		return Entries.size();
	}

	// Serialization.
	friend void Serialize( CSerializer& S, TMap<K, V>& Map )
	{
		Serialize( S, Map.Entries );
	}

	// Operators.
	Bool operator==( const TMap<K, V>& Other ) const
	{
		return Entries == Other.Entries;
	}
	Bool operator!=( const TMap<K, V>& Other ) const
	{
		return Entries != Other.Entries;
	}
	TMap<K, V>& operator=( const TMap<K, V>& Other )
	{
		Entries = Other.Entries;
		return *this;
	}

public:
	// Pair struct.
	class TEntry
	{
	public:
		K		Key;
		V		Value;
		TEntry()
		{}
		TEntry( const K& InKey, const V& InValue )
			:	Key(InKey), Value(InValue)
		{}
		friend void Serialize( CSerializer& S, TEntry& Ent )
		{
			Serialize( S, Ent.Key );
			Serialize( S, Ent.Value );
		}
	};

	// List of pairs.
	Array<TEntry>		Entries;

	// Find an index of entry by the key, if
	// no entry found return -1.
	Int32 FindEntry( const K& Key ) const
	{
		Int32 Low, High;
		for( Low=0, High=Entries.size(); Low<High; )
		{
			Int32 Middle = Low + (High-Low) / 2;
			if( Key <= Entries[Middle].Key )
				High	= Middle;
			else
				Low		= Middle+1;
		}
		return High<Entries.size() && Entries[High].Key==Key ? High : -1;
	}

	// Add a new entry to map.
	void Add( const K& Key, const V& Value )
	{
		Entries.setSize(Size()+1);
		Int32	i	= Size()-1;
		while( ( i>0 )&&( Key<Entries[i-1].Key ) )
		{
			Entries[i]	= Entries[i-1];
			i--;
		}
		Entries[i]	= TEntry( Key, Value );
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/