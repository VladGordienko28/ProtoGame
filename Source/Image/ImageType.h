//-----------------------------------------------------------------------------
//	ImageType.h: An image type
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace img
{
	/**
	 *	All supported image types
	 */
	enum class EImageType: UInt8
	{
		Grayscale,
		Palette,
		RGB,
		RGBA,
		RGB16,
		RGBA16,
		MAX
	};

	/**
	 *	An image
	 */
	class Image: public res::Resource
	{
	public:
		using Ptr = SharedPtr<Image>;
		static const auto RESOURCE_TYPE = res::EResourceType::Image;

		~Image();

		String getName() const { return m_name; }
		
		EImageType getType() const { return m_type; }

		UInt32 getUSize() const { return m_uSize; }
		UInt32 getVSize() const { return m_vSize; }

		UInt8 getUBits() const { return m_uBits; }
		UInt8 getVBits() const { return m_vBits; }

		rend::Texture2DHandle getHandle() const { return m_handle; }
		rend::ShaderResourceView getSRV() const { return m_srv; }

	private:
		String m_name;

		rend::Texture2DHandle m_handle;
		rend::ShaderResourceView m_srv;

		UInt32 m_uSize;
		UInt32 m_vSize;

		EImageType m_type;
		UInt8 m_uBits;
		UInt8 m_vBits;

		Image() = delete;
		Image( String name );

		Bool create( rend::Device* device, const res::CompiledResource& compiledResource );
		void destroy( rend::Device* device );

		friend class System;
	};
}
}