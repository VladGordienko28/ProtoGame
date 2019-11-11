//-----------------------------------------------------------------------------
//	Operations.h: An UI rendering operations
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
namespace rendering
{
	/**
	 *	A flat shaded rect draw operation
	 */
	struct FlatShadeOp
	{
	public:
		struct Vertex
		{
		public:
			math::Vector pos;
			math::Color color;
		};

		GrowOnlyArray<Vertex> vertices;
		GrowOnlyArray<UInt16> indices;
		Bool alphaEnabled;
	};

	/**
	 *	A image rect draw operation
	 */
	struct ImageOp
	{
	public:
		struct Vertex
		{
		public:
			math::Vector pos;
			math::Vector tc;
			math::Color color;
		};

		GrowOnlyArray<Vertex> vertices;
		GrowOnlyArray<UInt16> indices;
		rend::ShaderResourceView srv;
		// todo: add blend mode also
	};

	/**
	 *	A text draw operation
	 */
	struct TextOp
	{
	public:
		using Vertex = fnt::TextVertex;

		GrowOnlyArray<Vertex> vertices;
		GrowOnlyArray<UInt16> indices;
		rend::ShaderResourceView srv;
	};
}
}
}