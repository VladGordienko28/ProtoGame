//-----------------------------------------------------------------------------
//	Listener.h: A helper class which listening all resource manager messages
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A abstract class which listening all resource manager messages
	 */
	class IListener
	{
	public:
		virtual ~IListener() = default;

		virtual void onError( String resourceName, String message ) = 0;
		virtual void onWarning( String resourceName, String message ) = 0;
		virtual void onInfo( String resourceName, String message ) = 0;
	};

	/**
	 *	A list of listeners
	 */
	class ListenerList final: public IListener
	{
	public:
		ListenerList() = default;

		~ListenerList()
		{
			assert( m_listeners.size() == 0 );
		}

		void addListener( IListener* listener )
		{
			assert( listener );
			m_listeners.addUnique( listener );
		}

		void removeListener( IListener* listener )
		{
			assert( listener );
			m_listeners.removeUnique( listener, false );
		}

		void onError( String resourceName, String message ) override
		{
			for( auto& it : m_listeners )
			{
				it->onError( resourceName, message );
			}
		}

		void onWarning( String resourceName, String message ) override
		{
			for( auto& it : m_listeners )
			{
				it->onWarning( resourceName, message );
			}
		}

		void onInfo( String resourceName, String message ) override
		{
			for( auto& it : m_listeners )
			{
				it->onInfo( resourceName, message );
			}
		}

	private:
		Array<IListener*> m_listeners;
	};
}
}