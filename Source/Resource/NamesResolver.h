//-----------------------------------------------------------------------------
//	NamesResolver.h: A helper class which matches ResourceId to ResourceName
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A names resolver which matches ResourceId to 
	 *	ResourceName
	 */
	class NamesResolver final: NonCopyable
	{
	public:
		NamesResolver() = default;
		~NamesResolver() = default;

		void addName( ResourceId resourceId, String resourceName )
		{
			if( m_table.hasKey( resourceId ) )
			{
				String* oldResourceName = m_table.get( resourceId );

				if( resourceName != *oldResourceName )
				{
					fatal( TEXT( "Names hash collision for \"%s\" and \"%s\"" ),
						*resourceName, **oldResourceName );
				}

			}
			else
			{
				m_table.put( resourceId, resourceName );
			}
		}

		String getName( ResourceId resourceId ) const
		{
			const String* resourceName = m_table.get( resourceId );
			return resourceName ? *resourceName : TEXT( "" );
		}

	private:
		Map<ResourceId, String> m_table;
	};
}
}