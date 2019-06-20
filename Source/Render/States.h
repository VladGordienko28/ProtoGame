//-----------------------------------------------------------------------------
//	States.h: A basic rendering states
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace rend
{
	/**
	 *	A sampler filter
	 */
	enum class ESamplerFilter : UInt8
	{
		Point,
		Linear,
		MAX
	};

	/**
	 *	A sampler address mode
	 */
	enum class ESamplerAddressMode : UInt8
	{
		Wrap,
		Mirror,
		Clamp,
		MAX
	};

	/**
	 *	A sampler state identifier
	 */
	using SamplerStateId = UInt32;

	/**
	 *	A sampler state
	 */
	class SamplerState final
	{
	public:
		SamplerState( ESamplerFilter filter, ESamplerAddressMode addrMode = ESamplerAddressMode::Wrap )
			:	m_filter( filter ),
				m_addressMode( addrMode )
		{
		}

		~SamplerState()
		{
		}

		ESamplerFilter getFilter() const
		{
			return m_filter;
		}

		ESamplerAddressMode getAddressMode() const
		{
			return m_addressMode;
		}

		UInt32 getHash() const
		{
			return hashing::murmur32( this, sizeof( SamplerState ) );
		}

	private:
		ESamplerFilter m_filter;
		ESamplerAddressMode m_addressMode;

		SamplerState() = delete;
	};

	/**
	 *	A blending factor
	 */
	enum class EBlendFactor : UInt8
	{
		Zero,
		One,
		SrcColor,
		InvSrcColor,
		SrcAlpha,
		InvSrcAlpha,
		DestAlpha,
		InvDestAlpha,
		DestColor,
		InvDestColor,
		SrcAlphaSat,
		MAX
	};

	/**
	 *	A blending operator
	 */	
	enum class EBlendOp : UInt8
	{
		Add,
		Subtract,
		RevSubtract,
		Min,
		Max,
		MAX
	};

	/**
	 *	A blend state identifier
	 */	
	using BlendStateId = UInt32;

	/**
	 *	A blend state
	 */	
	class BlendState final
	{
	public:
		BlendState( EBlendFactor srcFactor, EBlendFactor destFactor, EBlendOp op,
			EBlendFactor srcAlphaFactor, EBlendFactor destAlphaFactor, EBlendOp alphaOp )
			:	m_srcFactor( srcFactor ),
				m_destFactor( destFactor ),
				m_op( op ),
				m_srcAlphaFactor( m_srcFactor ),
				m_destAlphaFactor( m_destAlphaFactor )
		{
		}

		~BlendState()
		{
		}

		EBlendFactor getSrcFactor() const
		{
			return m_srcFactor;
		}

		EBlendFactor getDestFactor() const
		{
			return m_destFactor;
		}

		EBlendOp getOp() const
		{
			return m_op;
		}

		EBlendFactor getSrcAlphaFactor() const
		{
			return m_srcAlphaFactor;
		}

		EBlendFactor getDestAlphaFactor() const
		{
			return m_destAlphaFactor;
		}

		EBlendOp getAlphaOp() const
		{
			return m_alphaOp;
		}

		UInt32 getHash() const
		{
			return hashing::murmur32( this, sizeof( BlendState ) );
		}

	private:
		EBlendFactor m_srcFactor;
		EBlendFactor m_destFactor;
		EBlendOp m_op;

		EBlendFactor m_srcAlphaFactor;
		EBlendFactor m_destAlphaFactor;
		EBlendOp m_alphaOp;

		BlendState() = delete;
	};
}
}