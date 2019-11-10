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


}
}
}