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
		static const SamplerStateId INVALID = -1;

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
			UInt32 hash = hashing::murmur32( this, sizeof( SamplerState ) );
			assert( hash != INVALID );

			return hash;
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
		static const BlendStateId INVALID = -1;

		BlendState( EBlendFactor srcFactor, EBlendFactor destFactor, EBlendOp op,
			EBlendFactor srcAlphaFactor, EBlendFactor destAlphaFactor, EBlendOp alphaOp )
			:	m_srcFactor( srcFactor ),
				m_destFactor( destFactor ),
				m_op( op ),
				m_srcAlphaFactor( srcAlphaFactor ),
				m_destAlphaFactor( destAlphaFactor ),
				m_alphaOp( alphaOp )
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
			UInt32 hash = hashing::murmur32( this, sizeof( BlendState ) );
			assert( hash != INVALID );

			return hash;
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

	/**
	 *	A comparsion function
	 */
	enum class EComparsionFunc : UInt8
	{
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always,
		MAX
	};

	/**
	 *	A stencil operation
	 */
	enum class EStencilOp : UInt8
	{
		Keep,
		Zero,
		Replace,
		IncSat,
		DecSat,
		Invert,
		Inc,
		Dec,
		MAX
	};

	/**
	 *	A depth-stencil state identifier
	 */	
	using DepthStencilStateId = UInt32;

	/**
	 *	A depth-stencil state
	 */	
	class DepthStencilState final
	{
	public:
		static const DepthStencilStateId INVALID = -1;

		DepthStencilState( Bool depthEnabled, Bool depthWriteEnabled, EComparsionFunc depthFunc,
			Bool stencilEnabled, EComparsionFunc stencilFunc, EStencilOp stencilPassOp, EStencilOp stencilFailOp )
			:	m_depthEnabled( depthEnabled ),
				m_depthWriteEnabled( depthWriteEnabled ),
				m_depthFunc( depthFunc ),
				m_stencilEnabled( stencilEnabled ),
				m_stencilFunc( stencilFunc ),
				m_stencilPassOp( stencilPassOp ),
				m_stencilFailOp( stencilFailOp )
		{	
		}

		~DepthStencilState()
		{
		}

		Bool isDepthEnabled() const
		{
			return m_depthEnabled;
		}

		Bool isDepthWriteEnabled() const
		{
			return m_depthWriteEnabled;
		}

		EComparsionFunc getDepthFunc() const
		{
			return m_depthFunc;
		}

		Bool isStencilEnabled() const
		{
			return m_stencilEnabled;
		}

		EComparsionFunc getStencilFunc() const
		{
			return m_stencilFunc;
		}

		EStencilOp getStencilPassOp() const
		{
			return m_stencilPassOp;
		}

		EStencilOp getStencilFailOp() const
		{
			return m_stencilFailOp;
		}

		UInt32 getHash() const
		{
			UInt32 hash = hashing::murmur32( this, sizeof( DepthStencilState ) );
			assert( hash != INVALID );
			
			return hash;
		}

	private:
		Bool m_depthEnabled;
		Bool m_depthWriteEnabled;
		EComparsionFunc m_depthFunc;

		Bool m_stencilEnabled;
		EComparsionFunc m_stencilFunc;
		EStencilOp m_stencilPassOp;
		EStencilOp m_stencilFailOp;

		DepthStencilState() = delete;
	};
}
}