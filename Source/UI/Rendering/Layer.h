//-----------------------------------------------------------------------------
//	Layer.h: An UI layer for rendering
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
namespace rendering
{
	/**
	 *	An UI layer of rendering opearations/batches
	 */
	class Layer: public NonCopyable
	{
	public:
		enum EPass
		{
			PASS_Early = 0,
			PASS_Late,
			PASS_MAX
		};

		Layer();
		~Layer();

		void create( rend::Device* device );
		void destroy();

		void clear();

		void generateFlatShadeBatches( FlatShadeStream& stream );
		void generateImageBatches(){}
		void generateTextBatches( TextStream& stream );

		void drawFlatShadeBatches( rend::Device* device, ffx::Effect::Ptr effect );
		void drawImageBatches( rend::Device* device, ffx::Effect::Ptr effect );
		void drawTextBatches( rend::Device* device, ffx::Effect::Ptr effect );

		Bool hasFlatShadeBatches() const
		{
			return m_flatShadeBatches.size() > 0;
		}

		Bool hasImageBatches() const
		{
			return m_imageBatches.size() > 0;
		}

		Bool hasTextBatches() const
		{
			return m_textBatches.size() > 0;
		}

		Canvas& getCanvas()
		{
			return m_canvas;
		}

	private:
		struct FlatShadeBatch
		{
		public:
			UInt32 firstIndex;
			UInt32 numIndices;
			Bool alphaEnabled;
		};

		struct ImageBatch
		{
		public:
			int stub;/////////////////////////////////////////////////
		};

		struct TextBatch
		{
		public:
			UInt32 firstIndex;
			UInt32 numIndices;
			rend::ShaderResourceView srv;
		};

		GrowOnlyArray<FlatShadeBatch> m_flatShadeBatches;
		GrowOnlyArray<ImageBatch> m_imageBatches;
		GrowOnlyArray<TextBatch> m_textBatches;

		rend::BlendStateId m_blendStateNone;
		rend::BlendStateId m_blendStateAlpha;

		rend::SamplerStateId m_samplerStateLinearWrap;

		Canvas m_canvas;
	};
}
}
}