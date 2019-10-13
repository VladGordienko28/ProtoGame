//-----------------------------------------------------------------------------
//	Types.h: Basic render types
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace rend
{
	/**
	 *	A resource usage flag
	 */
	enum class EUsage
	{
		Immutable,		// Non changable resource
		Dynamic,		// Dynamic changable resource( CPU write / GPU read )
		GPUOnly,		// Only GPU can read/write resource
		MAX
	};

	/**
	 *	A primitive topology
	 */
	enum class EPrimitiveTopology
	{
		Unknown,
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip
	};

	/**
	 *	Render resources handles
	 */
	using Texture1DHandle		= Handle<24, 8, 'TX1'>;
	using Texture2DHandle		= Handle<24, 8, 'TX2'>;
	using VertexBufferHandle	= Handle<24, 8, 'VTX'>;
	using IndexBufferHandle		= Handle<24, 8, 'IND'>;
	using ConstantBufferHandle	= Handle<24, 8, 'CNS'>;
	using ShaderHandle			= Handle<24, 8, 'SHR'>;
	using RenderTargetHandle	= Handle<24, 8, 'RTG'>;
	using DepthBufferHandle		= Handle<24, 8, 'DHP'>;

	/**
	 *	A rendering viewport bounds
	 */
	struct Viewport
	{
	public:
		Float x = 0.f;
		Float y = 0.f;
		Float width = 0.f;
		Float height = 0.f;
		Float minDepth = 0.f;
		Float maxDepth = 1.f;
	};

	/**
	 *	A shader resource view
	 */
	struct ShaderResourceView
	{
	public:
		// todo: rethink this, maybe template magic
		// will help me
		void* srv = nullptr;

		Bool operator==( const ShaderResourceView& other ) const
		{
			return srv == other.srv;
		}

		Bool operator!=( const ShaderResourceView& other ) const
		{
			return srv != other.srv;
		}
	};

	/**
	 *	An area which define scissor rect
	 */
	struct ScissorArea
	{
	public:
		Int32 top;
		Int32 left;
		Int32 bottom;
		Int32 right;

		ScissorArea()
		{
		}
		
		ScissorArea( Int32 inTop, Int32 inLeft, Int32 inBottom, Int32 inRight )
			:	top( inTop ), left( inLeft ),
				bottom( inBottom ), right( inRight )
		{
		}

		Bool operator==( const ScissorArea& other ) const
		{
			return top == other.top && left == other.left &&
						bottom == other.bottom && right == other.right;
		}

		Bool operator!=( const ScissorArea& other ) const
		{
			return top != other.top || left != other.left ||
						bottom != other.bottom || right != other.right;
		}

		static inline ScissorArea NULL_AREA()
		{
			return ScissorArea( -1, -1 ,-1, -1 );
		}
	};

	/**
	 *	A resource data format
	 */
	enum class EFormat
	{
		Unknown,

		R8_I,
		R8_U,

		R16_F,
		R16_I,
		R16_U,

		R32_F,
		R32_I,
		R32_U,

		RG8_I,
		RG8_U,

		RG16_F,
		RG16_I,
		RG16_U,

		RG32_F,
		RG32_I,
		RG32_U,

		RGB32_F,
		RGB32_I,
		RGB32_U,

		RGBA8_I,
		RGBA8_U,
		RGBA8_UNORM,

		RGBA32_F,
		RGBA32_I,
		RGBA32_U,

		D24S8,

		MAX
	};

	struct FormatInfo
	{
		const Char* name;
		UInt32 blockBytes;
		UInt32 blockSizeX;
		UInt32 blockSizeY;
		UInt32 componentsCount;
	};

	/**
	 *	Return information about specific format
	 */
	extern const FormatInfo& getFormatInfo( EFormat format );
	extern EFormat getFormatByName( String formatName );
}
}