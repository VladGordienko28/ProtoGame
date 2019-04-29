//-----------------------------------------------------------------------------
//	InterpCurve.h: An interpolation curve
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	An interpolation curve
	 */
	template<class T> class InterpCurve
	{
	public:
		struct Sample
		{
			Float input;
			T output;
		};

		InterpCurve()
		{
		}

		InterpCurve( const InterpCurve<T>& other )
			:	m_samples( other.m_samples )
		{
		}

		~InterpCurve()
		{
			empty();
		}

		/**
		 *	Insert a new sample to the curve and returns it's index
		 */
		Int32 addSample( Float input, const T& output )
		{
			// find appropriated place in the sorted list of samples
			Int32 i;
			for( i = 0; i < m_samples.size() && m_samples[i].input < input; ++i );

			// insert
			m_samples.insert( i, 1 );
			m_samples[i] = { input, output };
			return i;
		}

		/**
		 *	Remove an i-th sample from the sorted array of the
		 *	samples
		 */
		void removeSample( Int32 i )
		{
			m_samples.removeShift( i );
		}

		/**
		 *	Move an i-th sample to somewhere else, return a new
		 *	index of the old sample
		 */
		Int32 moveSample( Int32 i, Float newInput )
		{
			T oldOutput = m_samples[i].output;
			removeSample( i );
			return addSample( newInput, oldOutput );
		}

		void empty()
		{
			m_samples.empty();
		}

		Int32 numSamples() const
		{
			return m_samples.size();
		}

		const Sample& getSample( Int32 i ) const
		{
			return m_samples[i];
		}

		T& getOutputOf( Int32 i )
		{
			return m_samples[i].output;
		}

		T sampleStepped( Float input, const T& _default ) const
		{
			if( m_samples.size() == 0 )
			{
				return _default;
			}
			else if( m_samples.size() < 2 || input <= m_samples[0].input )
			{
				return m_samples[0].output;
			}
			else if( input >= m_samples[m_samples.size() - 1].input  )
			{
				return m_samples[m_samples.size() - 1].output;
			}
			else
			{
				for( Int32 i = 1; i < m_samples.size(); ++i )
				{
					const Sample& a = m_samples[i - 1];
					const Sample& b = m_samples[i];

					if( b.input >= input )
					{
						return a.output;
					}
				}

				return _default;
			}
		}

		T sampleLinear( Float input, const T& _default ) const
		{
			if( m_samples.size() == 0 )
			{
				return _default;
			}
			else if( m_samples.size() < 2 || input <= m_samples[0].input )
			{
				return m_samples[0].output;
			}
			else if( input >= m_samples[m_samples.size() - 1].input  )
			{
				return m_samples[m_samples.size() - 1].output;
			}
			else
			{
				for( Int32 i = 1; i < m_samples.size(); ++i )
				{
					const Sample& a = m_samples[i - 1];
					const Sample& b = m_samples[i];

					if( b.input >= input )
					{
						Float alpha = ( input - a.input ) / ( b.input - a.input );
						return lerp( a.output, b.output, alpha );
					}
				}

				return _default;
			}
		}

		// todo: add more sample functions

	protected:
		Array<Sample> m_samples;
	};
}
}