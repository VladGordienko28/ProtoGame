//-----------------------------------------------------------------------------
//	UIRender.h: An UI render
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An UI render
	 */
	class Render: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<Render>;

		virtual ~Render() = default;

		virtual void prepareBatches( Container* treeRoot ) = 0;
		virtual void flushBatches() = 0;

		static Render* createRender( rend::Device* device );
	};
}
}