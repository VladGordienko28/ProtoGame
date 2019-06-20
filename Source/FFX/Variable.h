//-----------------------------------------------------------------------------
//	Variable.h: A ffx variable
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 * A variable flags
	 */
	enum EVariableFlags : UInt32
	{
		VAR_None = 0x0000,
	};

	/**
	 * A variable in the shader
	 */
	class Variable final: NonCopyable
	{
	public:
		Variable( UInt32 size, UInt32 dimension, UInt32 offset, BufferHandle buffer );
		Variable( JSon::Ptr node );
		~Variable();

		UInt32 getFlags() const
		{
			return m_flags;
		}

		void setFlags( UInt32 flags )
		{
			m_flags |= flags;
		}

		void clearFlags( UInt32 flags )
		{
			m_flags &= ~flags;
		}

		UInt32 getSize() const
		{
			return m_size;
		}

		UInt32 getDimension() const
		{
			return m_dimension;
		}

		UInt32 getOffset() const
		{
			return m_offset;
		}

		BufferHandle getBuffer() const
		{
			return m_buffer;
		}

	private:
		UInt32 m_size = 0;
		UInt32 m_dimension = 0;
		UInt32 m_offset = 0;

		UInt32 m_flags = EVariableFlags::VAR_None;
		BufferHandle m_buffer;

		Variable() = delete;
	};
}
}