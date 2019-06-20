//-----------------------------------------------------------------------------
//	Variable.cpp: A ffx variable implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	Variable::Variable( JSon::Ptr node )
	{
		assert( false && "Not implemented yet" );
	}

	Variable::Variable( UInt32 size, UInt32 dimension, UInt32 offset, BufferHandle buffer )
		:	m_size( size ),
			m_dimension( dimension ),
			m_offset( offset ),
			m_buffer( buffer )
	{
	}

	Variable::~Variable()
	{
	}
}
}