//-----------------------------------------------------------------------------
//	DxTypes.h: A DirectX 11 types wrappers
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace dx11
{
	/**
	 *	A DirectX 11 1D Texture
	 */
	struct DxTexture1D
	{
	public:
		DxRef<ID3D11Texture1D> m_texture = nullptr;
		DxRef<ID3D11ShaderResourceView> m_srv = nullptr;
		Int32 m_width = 0;
		Int32 m_levels = 0;
		rend::EUsage m_usage = rend::EUsage::Immutable;
		rend::EFormat m_format = rend::EFormat::Unknown;

		DxTexture1D() = default;
		~DxTexture1D();

		Bool create( ID3D11Device* device, rend::EFormat format, Int32 width, Int32 mips, 
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName );

		void destroy( ID3D11Device* device );
	};

	/**
	 *	A DirectX 11 2D Texture
	 */
	struct DxTexture2D
	{
	public:
		DxRef<ID3D11Texture2D> m_texture = nullptr;
		DxRef<ID3D11ShaderResourceView> m_srv = nullptr;
		Int32 m_width = 0;
		Int32 m_height = 0;
		Int32 m_levels = 0;
		rend::EUsage m_usage = rend::EUsage::Immutable;
		rend::EFormat m_format = rend::EFormat::Unknown;

		DxTexture2D() = default;
		~DxTexture2D();

		Bool create( ID3D11Device* device, rend::EFormat format, Int32 width, Int32 height, Int32 mips, 
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName );

		void destroy( ID3D11Device* device );
	};

	/**
	 *	A DirectX 11 Render Target
	 */
	struct DxRenderTarget
	{
	public:
		DxRef<ID3D11Texture2D> m_texture = nullptr;
		DxRef<ID3D11ShaderResourceView> m_srv = nullptr;
		DxRef<ID3D11RenderTargetView> m_rtv = nullptr;
		Int32 m_width = 0;
		Int32 m_height = 0;
		rend::EFormat m_format = rend::EFormat::Unknown;

		DxRenderTarget() = default;
		~DxRenderTarget();

		Bool create( ID3D11Device* device, rend::EFormat format, Int32 width, Int32 height, const AnsiChar* debugName );
		void destroy( ID3D11Device* device );
	};

	/**
	 *	A DirectX 11 Pixel Shader
	 */
	struct DxPixelShader
	{
	public:
		DxRef<ID3D11PixelShader> m_shader = nullptr;

		DxPixelShader() = default;
		~DxPixelShader();

		Bool create( ID3D11Device* device, const rend::CompiledShader& shader, const AnsiChar* debugName );
		void destroy( ID3D11Device* device );
	};

	/**
	 *	A DirectX 11 Vertex Shader
	 */
	struct DxVertexShader
	{
	public:
		DxRef<ID3D11VertexShader> m_shader = nullptr;
		UInt32 m_vertexDeclaration = 0;

		DxVertexShader() = default;
		~DxVertexShader();

		Bool create( ID3D11Device* device, const rend::CompiledShader& shader, UInt32 vertexDeclaration,
			const AnsiChar* debugName );
		void destroy( ID3D11Device* device );
	};

	/**
	 *	A DirectX 11 Vertex Buffer
	 */
	struct DxVertexBuffer
	{
	public:
		DxRef<ID3D11Buffer> m_buffer = nullptr;
		rend::EUsage m_usage = rend::EUsage::Immutable;
		UInt32 m_vertexSize = 0;
		UInt32 m_numVerts = 0;

		DxVertexBuffer() = default;
		~DxVertexBuffer();

		Bool create( ID3D11Device* device, UInt32 vertexSize, UInt32 numVerts, 
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName );

		void destroy( ID3D11Device* device );
	};

	/**
	 *	A DirectX 11 Index Buffer
	 */
	struct DxIndexBuffer
	{
	public:
		DxRef<ID3D11Buffer> m_buffer = nullptr;
		DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
		rend::EUsage m_usage = rend::EUsage::Immutable;
		UInt32 m_numIndexes = 0;

		DxIndexBuffer() = default;
		~DxIndexBuffer();

		Bool create( ID3D11Device* device, rend::EFormat format, UInt32 numIndexes,
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName );

		void destroy( ID3D11Device* device );
	};

	/**
	 *	A DirectX 11 Constant Buffer
	 */
	struct DxConstantBuffer
	{
	public:
		DxRef<ID3D11Buffer> m_buffer = nullptr;
		rend::EUsage m_usage = rend::EUsage::Immutable;
		UInt32 m_size = 0;

		DxConstantBuffer() = default;
		~DxConstantBuffer();

		Bool create( ID3D11Device* device, UInt32 bufferSize,
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName );

		void destroy( ID3D11Device* device );
	};
}
}