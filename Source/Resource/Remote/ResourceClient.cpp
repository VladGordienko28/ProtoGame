//-----------------------------------------------------------------------------
//	RemoteClient.cpp: A remote resource client implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource/Resource.h"

namespace flu
{
namespace res
{
	ResourceClient::ResourceClient( String appName, const net::Address& serverAddress )
	{
		m_tcpClient = net::NetworkManager::createTCPClient();
		auto errorCode = m_tcpClient->connect( serverAddress );

		if( errorCode != net::EError::Ok )
		{
			fatal( L"Unable to connect to Resource Server at %s", *serverAddress.toString() );
		}

		// wait for the connection to be established
		while( m_tcpClient->getState() == net::TCPClient::EState::Connecting )
		{
			errorCode = m_tcpClient->waitForConnection();
			
			if( errorCode != net::EError::Ok )
			{
				fatal( L"Resource Server %s is not available", *serverAddress.toString() );
			}
			
			threading::sleep( NET_IDLING_MS );
		}

		info( L"Connected to Resource Server at %s", *serverAddress.toString() );

		// wait for server verification
		{
			ServerMessageHeader header;
			Array<UInt8> data;

			receiveServerMessage( header, data );

			if( header.message != EServerMessage::RequestInfo )
			{
				fatal( L"Unexpected server request" );
			}
		}

		// send client data
		{
			OwningBufferWriter writer;
			writer << appName;

			sendClientMessage( EClientMessage::ProvideInfo, writer.getData(), writer.size() );
		}

		info( L"Resource Server accepted this client" );
	}

	ResourceClient::~ResourceClient()
	{
		if( m_tcpClient.hasObject() )
		{
			m_tcpClient->shutdown();
			m_tcpClient = nullptr;
		}
	}

	Array<ResourceId> ResourceClient::trackChanges()
	{
		// check if something changed outside
		ServerMessageHeader header;
		Array<UInt8> data;

		if( receiveServerMessage( header, data, false ) == EReceiveResult::Ok )
		{
			assert( header.message == EServerMessage::NotifyReload );

			BufferReader reader( data );

			Array<ResourceId> changedResources;
			reader >> changedResources;

			return changedResources;
		}

		return Array<ResourceId>();
	}

	CompiledResource ResourceClient::requestCompiled( EResourceType type, String resourceName )
	{ 
		OwningBufferWriter writer;
		writer << type;
		writer << resourceName;

		sendClientMessage( EClientMessage::RequestResourceByName, writer.getData(), writer.size() );
		return receiveResource();
	}

	CompiledResource ResourceClient::requestCompiled( ResourceId resourceId )
	{
		OwningBufferWriter writer;
		writer << resourceId;

		sendClientMessage( EClientMessage::RequestResourceById, writer.getData(), writer.size() );
		return receiveResource();
	}

	String ResourceClient::resolveResourceId( ResourceId resourceId )
	{
		sendClientMessage( EClientMessage::ResolveName, &resourceId, sizeof( ResourceId ) );
		
		ServerMessageHeader header;
		Array<UInt8> data;

		receiveServerMessage( header, data );

		BufferReader reader( data );
		String resourceName;

		reader >> resourceName;

		return resourceName;
	}

	CompiledResource ResourceClient::receiveResource()
	{
		CompiledResource compiledResource;

		Int32 resourceSize = 0;
		Int32 resourceBytesTransferred = 0;

		while( true )
		{
			// read block of resource data
			{
				ServerMessageHeader header;
				Array<UInt8> data;

				receiveServerMessage( header, data );

				BufferReader reader( data );

				if( header.message == EServerMessage::ResourceError )
				{
					// resource missing
					return CompiledResource();
				}
				else if( header.message == EServerMessage::ProvideResource && resourceSize == 0 )
				{
					reader >> resourceSize;
					compiledResource.data.setSize( resourceSize );
				}
				else if( header.message == EServerMessage::NextResourceBlock && resourceSize > 0 )
				{
				}
				else
				{
					fatal( L"Unexpected server respond %d", header.message );
				}

				// insert new data to compiled resource
				UInt32 bytesReceived = reader.totalSize() - reader.tell();
				reader.readData( &compiledResource.data[resourceBytesTransferred], bytesReceived );

				resourceBytesTransferred += bytesReceived;

				if( resourceBytesTransferred >= resourceSize )
				{
					assert( resourceBytesTransferred == resourceSize );
					break;
				}
			}

			// write new request
			sendClientMessage( EClientMessage::RequestNextBlock, nullptr, 0 );
		}

		return compiledResource;
	}

	ResourceClient::EReceiveResult ResourceClient::receiveServerMessage( ServerMessageHeader& header, Array<UInt8>& data, Bool blocking )
	{
		SizeT bytesReceived;

		// kind of blocking socket
		if( m_tcpClient->receiveData( &header, sizeof( ServerMessageHeader ), bytesReceived, blocking ) == net::EError::Ok )
		{
			if( bytesReceived > 0 )
			{
				assert( bytesReceived == sizeof( ServerMessageHeader ) );

				bytesReceived = 0;
				data.setSize( header.payloadSize );

				if( header.payloadSize == 0 || m_tcpClient->receiveData( &data[0], header.payloadSize, bytesReceived, true ) == net::EError::Ok )
				{
					assert( bytesReceived == header.payloadSize );
					return EReceiveResult::Ok;
				}
				else
				{
					fatal( L"Unexpected end of server request" );
				}
			}
			else if( !blocking )
			{
				return EReceiveResult::Pending;
			}
		}
		else
		{
			// unable to read header
			fatal( L"Resource Server was disconnected" );
		}
	}

	void ResourceClient::sendClientMessage( EClientMessage message, const void* data, SizeT dataSize )
	{
		assert( dataSize < MAX_PACKET_SIZE );

		ClientMessageHeader header;
		header.message = message;
		header.payloadSize = static_cast<UInt16>( dataSize );

		OwningBufferWriter writer;
		writer.writeData( &header, sizeof( ClientMessageHeader ) );
		
		if( data && dataSize > 0 )
		{
			writer.writeData( data, dataSize );
		}

		SizeT bytesSended;
		if( m_tcpClient->sendData( writer.getData(), writer.size(), bytesSended, true ) == net::EError::Ok )
		{
			assert( bytesSended == writer.size() );
		}
		else
		{
			// send failed
			fatal( L"Server connection was interrupted" );
		}
	}
}
}