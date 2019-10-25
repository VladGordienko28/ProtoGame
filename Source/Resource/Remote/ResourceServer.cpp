//-----------------------------------------------------------------------------
//	RemoteServer.cpp: A remote resource server implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource/Resource.h"

namespace flu
{
namespace res
{
	Bool ResourceServer::create()
	{
		assert( !instance().m_isInitialized );

		// open server connection
		UInt16 port = ConfigManager::readInt( EConfigFile::Application, TXT("ResourceServer"), 
			TXT("Port"), DEFAULT_LISTENING_PORT );

		instance().m_tcpServer = net::NetworkManager::createTCPServer();
		if( instance().m_tcpServer->listen( port, 5 ) != net::EError::Ok )
		{
			error( L"Unable to start Resource Server at port %d", port );
			return false;
		}

		// create local resource storage
		String packagesPath = ConfigManager::readString( EConfigFile::Application, TXT("ResourceManager"), TXT("PackagesPath") );
		String cachePath = ConfigManager::readString( EConfigFile::Application, TXT("ResourceManager"), TXT("CachePath") );
		Bool useCache = ConfigManager::readBool( EConfigFile::Application, TXT("ResourceManager"), TXT("UseCache") );

		instance().m_localStorage = new LocalStorage( packagesPath, cachePath, useCache );
		instance().m_localStorage->setListener( &instance().m_logListener );

		String hostName;
		net::Address addr = net::NetworkManager::getLocalIP( &hostName );
		addr.port = port;

		instance().m_timeToTrackRemain = FILES_TRACKING_PERIOD_SEC;
		instance().m_lastTickTime = time::cycles64();

		info( L"Resource Server started successfully at \"%s\" %s", *hostName, *addr.toString() );
		instance().m_isInitialized = true;
		return true;
	}

	void ResourceServer::destroy()
	{
		assert( instance().m_isInitialized );

		instance().m_tcpServer->shutdown();
		instance().m_tcpServer = nullptr;
		instance().m_clients.empty();

		instance().m_localStorage = nullptr;

		instance().m_isInitialized = false;
	}

	void ResourceServer::registerResourceType( EResourceType type, IResourceCompiler* compiler )
	{
		instance().m_localStorage->registerCompiler( type, compiler ); 
	}

	Bool ResourceServer::processRequests()
	{
		assert( instance().m_isInitialized );

		Bool wasProcessed = false;

		// poll new clients
		instance().pollClients();

		// track changed files
		UInt64 thisTickTime = time::cycles64();
		Double deltaTime = time::cyclesToSec( thisTickTime - instance().m_lastTickTime );
		instance().m_lastTickTime = thisTickTime;

		Array<ResourceId> changedResources;
		if( ( instance().m_timeToTrackRemain -= deltaTime ) < 0.0 )
		{
			// time to track
			changedResources = instance().m_localStorage->trackChanges();

			if( changedResources.size() > 0 )
			{
				info( L"%d resources were changed...", changedResources.size() );
			}

			instance().m_timeToTrackRemain = FILES_TRACKING_PERIOD_SEC;
		}

		// serve all clients, warning client may disconnect
		for( Int32 i = 0; i < instance().m_clients.size();  )
		{
			Client& client = instance().m_clients[i];

			if( instance().serveClient( client, changedResources, wasProcessed ) )
			{
				// client served successfully
				++i;
			}
			else
			{
				// client was disconnected
				warn( L"Client: \"%s\" %s was disconnected", *client.name, *client.address.toString() );
				instance().m_clients.removeShift( i );
				
				wasProcessed = true;
			}
		}

		return wasProcessed;
	}

	ResourceServer::ResourceServer()
		:	m_isInitialized( false ),
			m_localStorage( nullptr ),
			m_tcpServer( nullptr ),
			m_clients(),
			m_lastTickTime( 0 ),
			m_timeToTrackRemain( 0.f )
	{
	}

	ResourceServer::~ResourceServer()
	{
		if( m_isInitialized )
		{
			fatal( TXT( "ResourceServer wasn't deinitialized properly" ) );
		}
	}

	Bool ResourceServer::serveClient( Client& client, const Array<ResourceId>& changedResources, Bool& wasProcessed )
	{
		switch( client.status )
		{
			case Client::EStatus::Unverified:
				return serveUnverified( client, wasProcessed );

			case Client::EStatus::Accepted:
				return serveAccepted( client, changedResources, wasProcessed );

			default:
				assert( false && "Unknown client status" );
				return false;
		}
	}

	Bool ResourceServer::serveUnverified( Client& client, Bool& wasProcessed )
	{
		ClientMessageHeader header;
		Array<UInt8> data;

		EReceiveResult result = receiveClientMessage( client.id, header, data, false ); 

		if( result == EReceiveResult::Ok )
		{
			if( header.message != EClientMessage::ProvideInfo )
			{
				warn( L"Unexpected message from %s", *client.address.toString() );
				return false;
			}

			BufferReader reader( data );
			String clientInfo;
			reader >> clientInfo;

			info( L"Client: \"%s\" %s accepted", *clientInfo, *client.address.toString() );
			client.status = Client::EStatus::Accepted;

			wasProcessed = true;
			return true;
		}
		else
		{
			// pending or failed
			return result == EReceiveResult::Pending;
		}
	}

	Bool ResourceServer::serveAccepted( Client& client, const Array<ResourceId>& changedResources, Bool& wasProcessed )
	{
		// Handle all requests
		{
			ClientMessageHeader header;
			Array<UInt8> data;

			EReceiveResult result = receiveClientMessage( client.id, header, data, false );

			if( result == EReceiveResult::Ok )
			{
				wasProcessed = true;

				switch( header.message )
				{
					case EClientMessage::RequestResourceById:
					case EClientMessage::RequestResourceByName:
						return handleRequestResource( client, header, data );

					case EClientMessage::RequestNextBlock:
						return handleRequestBlock( client, header, data );

					case EClientMessage::ResolveName:
						return handleResolveName( client, header, data );

					default:
						warn( L"Unexpected message from %s", *client.address.toString() );
						return false;
				}
			}
			else if( result == EReceiveResult::Failed )
			{
				return false;
			}
		}

		// Send changes notification
		if( changedResources.size() > 0 )
		{
			wasProcessed = true;

			OwningBufferWriter writer;
			writer << changedResources;

			assert( writer.size() <= MAX_PACKET_SIZE - sizeof( ServerMessageHeader ) );
			return sendServerMessage( client.id, EServerMessage::NotifyReload, writer.getData(), writer.size() );
		}

		return true;
	}

	Bool ResourceServer::handleRequestResource( Client& client, const ClientMessageHeader& reqHeader, const Array<UInt8>& reqData )
	{
		BufferReader reader( reqData );

		CompiledResource compiledResource;
		ListenerList nullListener; // todo: get rid of it!!

		// try to compile or load
		if( reqHeader.message == EClientMessage::RequestResourceById )
		{
			ResourceId resId;
			reader >> resId;

			info( L"Client: %s requested resource %s", *client.name, *resId.toString() );
			compiledResource = m_localStorage->requestCompiled( resId );
		}
		else if( reqHeader.message == EClientMessage::RequestResourceByName )
		{
			EResourceType resType;
			String resName;

			reader >> resType;
			reader >> resName;

			info( L"Client: %s requested resource \"%s\"", *client.name, *resName );
			compiledResource = m_localStorage->requestCompiled( resType, resName );
		}

		assert( client.resourceBytesRemain == 0 );

		if( compiledResource.isValid() )
		{
			client.resourceData = compiledResource.data;
			client.resourceBytesRemain = compiledResource.data.size();
			client.transferStartTime = time::cycles64();

			OwningBufferWriter writer;
			writer << compiledResource.data.size();

			Int32 bytesToSend = min<Int32>( client.resourceBytesRemain, MAX_PACKET_SIZE - writer.size() - sizeof( ServerMessageHeader ) );
			assert( bytesToSend <= MAX_PACKET_SIZE );

			writer.writeData( &client.resourceData[client.resourceData.size() - client.resourceBytesRemain], bytesToSend );

			if( sendServerMessage( client.id, EServerMessage::ProvideResource, writer.getData(), writer.size() ) )
			{	
				info( L"\ttransferring: %d/%d bytes..", client.resourceData.size() - client.resourceBytesRemain, client.resourceData.size() );
				client.resourceBytesRemain -= bytesToSend;

				if( client.resourceBytesRemain == 0 )
				{
					info( L"\tdone in %.4f sec", time::elapsedSecFrom( client.transferStartTime ) );
				}
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			warn( L"Client: %s request denied", *client.name );
			return sendServerMessage( client.id, EServerMessage::ResourceError, nullptr, 0 );
		}
	}

	Bool ResourceServer::handleRequestBlock( Client& client, const ClientMessageHeader& reqHeader, const Array<UInt8>& reqData )
	{
		if( client.resourceData.size() > 0 && client.resourceBytesRemain > 0 )
		{
			Int32 bytesToSend = min<Int32>( client.resourceBytesRemain, MAX_PACKET_SIZE - sizeof( ServerMessageHeader ) );
			assert( bytesToSend <= MAX_PACKET_SIZE );

			if( sendServerMessage( client.id, EServerMessage::NextResourceBlock, 
				&client.resourceData[client.resourceData.size() - client.resourceBytesRemain], bytesToSend ) )
			{
				info( L"\ttransferring: %d/%d bytes..", client.resourceData.size() - client.resourceBytesRemain, client.resourceData.size() );

				client.resourceBytesRemain -= bytesToSend;
				assert( client.resourceBytesRemain >= 0 );

				if( client.resourceBytesRemain == 0 )
				{
					info( L"\tdone in %.4f sec", time::elapsedSecFrom( client.transferStartTime ) );
				}

				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			error( L"Client: Resource block requests overflowed for %s", *client.name );
			return false;
		}
	}

	Bool ResourceServer::handleResolveName( Client& client, const ClientMessageHeader& reqHeader, const Array<UInt8>& reqData )
	{
		BufferReader reader( reqData );
		
		ResourceId resourceId;
		reader >> resourceId;

		String resourceName = m_localStorage->resolveResourceId( resourceId );

		// todo: use here and everywere allocations on stack!!
		OwningBufferWriter writer;
		writer << resourceName;

		info( L"Client: %s requested resolve %s -> \"%s\"", *client.name, *resourceId.toString(), *resourceName );

		return sendServerMessage( client.id, EServerMessage::ResolvedName, writer.getData(), writer.size() );
	}

	ResourceServer::EReceiveResult ResourceServer::receiveClientMessage( net::TCPServer::ClientId id,
		ClientMessageHeader& header, Array<UInt8>& data, Bool blocking )
	{
		SizeT bytesReceived;

		if( m_tcpServer->receiveData( id, &header, sizeof( ClientMessageHeader ), bytesReceived, blocking ) == net::EError::Ok )
		{
			if( bytesReceived > 0 )
			{
				assert( bytesReceived == sizeof( ClientMessageHeader ) );

				// read data
				bytesReceived = 0;
				data.setSize( header.payloadSize );

				if( header.payloadSize == 0 || 
					m_tcpServer->receiveData( id, &data[0], header.payloadSize, bytesReceived, true ) == net::EError::Ok )
				{
					assert( bytesReceived == header.payloadSize );
					return EReceiveResult::Ok;
				}
				else
				{
					// unable to receive data
					return EReceiveResult::Failed;
				}
			}
			else
			{
				// nothing to read yet
				return EReceiveResult::Pending;
			}
		}
		else
		{
			// unable to read header
			return EReceiveResult::Failed;
		}
	}

	Bool ResourceServer::sendServerMessage( net::TCPServer::ClientId id, EServerMessage message, const void* data, SizeT dataSize )
	{
		assert( dataSize < MAX_PACKET_SIZE );

		ServerMessageHeader header;
		header.message = message;
		header.payloadSize = static_cast<UInt16>( dataSize );

		OwningBufferWriter writer;
		writer.writeData( &header, sizeof( ServerMessageHeader ) );
		
		if( data && dataSize > 0 )
		{
			writer.writeData( data, dataSize );
		}

		SizeT bytesSended;
		if( m_tcpServer->sendData( id, writer.getData(), writer.size(), bytesSended, true ) == net::EError::Ok )
		{
			assert( bytesSended == writer.size() );
			return true;
		}
		else
		{
			// send failed
			return false;
		}
	}

	UInt32 ResourceServer::pollClients()
	{
		auto incomingClientId = m_tcpServer->pollConnection();

		if( incomingClientId != INVALID_HANDLE<net::TCPServer::ClientId>() )
		{
			// add a new client
			Client client;
			client.id = incomingClientId;
			client.address = m_tcpServer->getClientAddress( incomingClientId );
			client.name = net::NetworkManager::unresolveIP( client.address );
			client.status = Client::EStatus::Unverified;

			// send request for verification
			if( sendServerMessage( client.id, EServerMessage::RequestInfo, nullptr, 0 ) )
			{
				m_clients.push( client );
				
				info( L"Connected a new client \"%s\" %s", *client.name, *client.address.toString() );
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	ResourceServer& ResourceServer::instance()
	{
		static ResourceServer resourceServer;
		return resourceServer;
	}
}
}