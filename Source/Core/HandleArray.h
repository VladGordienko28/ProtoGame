//-----------------------------------------------------------------------------
//	HandleArray.h: A simple array which is using handles instead of indexes
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	.................
	 */
	template<SizeT ID_BITS, SizeT GEN_BITS, const Char* DESCR> class Handle
	{
	public:



	private:
		UInt32 m_id : ID_BITS;
		UInt32 m_generation : GEN_BITS;


		//friend template<typename HANDLE_TYPE, SizeT MAX_SIZE> class HandleArray;

		static_assert( DESCR != nullptr, "Handle description shouldn't be null" );
		static_assert( ID_BITS + GEN_BITS == 32, "Total Handle bits should be 32" );
		static_assert( sizeof( UInt32 ) == sizeof( Handle<ID_BITS, GEN_BITS, DESCR> ), 
			"Size of Handle should be the same UInt32" );
	};

	/**
	 *	.................
	 */
	template<typename HANDLE_TYPE, typename T, SizeT MAX_SIZE> class HandleArray
	{
	public:




	private:
		T m_data[MAX_SIZE];
		UInt32 m_firstAvailable;

		static_assert( isPowerOfTwo( MAX_SIZE ), "MaxSize of HandleArray should be power of two" );
	};
}