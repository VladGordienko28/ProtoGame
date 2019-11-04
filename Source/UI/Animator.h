//-----------------------------------------------------------------------------
//	Animator.h: An UI element animator
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An animation blend type
	 */
	enum class EBlendType
	{
		Override,		// play a new animation from the current pose
		Continue,		// play a new animation as the rest of the previous
		ContinueInv,	// play a new animation with the same duration as previous
		Reset			// play a new animation from the ref pose
	};

	/**
	 *	An UI element animator
	 */
	class Animator: public NonCopyable
	{
	public:
		using Callback = void(*)( Element* element );

		Animator( Element* element );
		~Animator();

		void update( Float delta );
		void setRefPose( const Position& position, const Size& size, Float opacity );

		void play( const Animation& animation, EBlendType blendType, Callback callback = nullptr );

		Bool isPlaying() const
		{
			return m_playing;
		}

	private:
		Element* m_element;
		AnimKey m_refKey;

		AnimKey m_targetKey;
		AnimKey m_currentKey;
		AnimKey m_startKey;

		Callback m_finishCallback;
		Float m_delayRemainTime;
		Float m_animDuration;
		Float m_animRemainTime;
		Bool m_playing;

		Animator() = delete;
	};
}
}