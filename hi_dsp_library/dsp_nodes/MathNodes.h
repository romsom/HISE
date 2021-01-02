/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#pragma once

namespace scriptnode {
using namespace juce;
using namespace hise;


namespace math
{
namespace Operations
{
#define SET_ID(x) static Identifier getId() { RETURN_STATIC_IDENTIFIER(#x); }
#define SET_DEFAULT(x) static constexpr float defaultValue = x;


#define OP_BLOCK(data, value) template <typename PD> static void op(PD& data, float value)
#define OP_SINGLE(data, value) template <typename FD> static void opSingle(FD& data, float value)
#define OP_BLOCK2SINGLE(data, value) OP_BLOCK(data, value) { for (auto ch : data) { opSingle(data.toChannelData(ch), value); }}

	struct mul
	{
		SET_ID(mul); SET_DEFAULT(1.0f);

		OP_BLOCK(data, value)
		{
			for (auto ch : data)
				hmath::vmuls(data.toChannelData(ch), value);
		}

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s *= value;
		}
	};

	struct add
	{
		SET_ID(add); SET_DEFAULT(0.0f);

		OP_BLOCK(data, value)
		{
			for (auto ch : data)
			{
				block b(data.toChannelData(ch));
				hmath::vadds(b, value);
			}
				
		}

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s += value;
		}
	};

	struct clear
	{
		SET_ID(clear); SET_DEFAULT(0.0f);

		OP_BLOCK(data, value)
		{
			for (auto ch : data)
				hmath::vset(data.toChannelData(ch), 0.0f);
		}

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s = 0.0f;
		}
	};

	struct sub
	{
		SET_ID(sub); SET_DEFAULT(0.0f);

		OP_BLOCK(data, value)
		{
			for (auto ch : data)
				hmath::vadds(data.toChannelData(ch), -value);
		}

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s -= value;
		}
	};

	struct div
	{
		SET_ID(div); SET_DEFAULT(1.0f);

		OP_BLOCK(data, value)
		{
			auto factor = value > 0.0f ? 1.0f / value : 0.0f;

			for (auto ch : data)
				hmath::vmuls(data.toChannelData(ch), factor);
		}

		OP_SINGLE(data, value)
		{
			auto factor = value > 0.0f ? 1.0f / value : 0.0f;

			for (auto& s : data)
				s *= factor;
		}
	};



	struct tanh
	{
		SET_ID(tanh); SET_DEFAULT(1.0f);

		OP_BLOCK2SINGLE(data, value);

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s = tanhf(s * value);
		}
	};

	struct pi
	{
		SET_ID(pi); SET_DEFAULT(2.0f);

		OP_BLOCK(data, unused)
		{
			for (auto ch : data)
				hmath::vmuls(data.toChannelData(ch), float_Pi);
		}

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s *= float_Pi;
		}
	};

	struct sin
	{
		SET_ID(sin); SET_DEFAULT(2.0f);

		OP_BLOCK2SINGLE(data, unused);

		OP_SINGLE(data, unused)
		{
			for (auto& s : data)
				s = sinf(s);
		}
	};

	struct sig2mod
	{
		SET_ID(sig2mod); SET_DEFAULT(0.0f);

		OP_BLOCK2SINGLE(data, unused);

		OP_SINGLE(data, unused)
		{
			for (auto& s : data)
				s = s * 0.5f + 0.5f;
		}
	};

	struct clip
	{
		SET_ID(clip); SET_DEFAULT(1.0f);

		OP_BLOCK(data, value)
		{
			for (auto ch : data)
				hmath::vclip(data.toChannelData(ch), -value, value);
		}

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s *= jlimit(-value, value, s);
		}
	};

	struct square
	{
		SET_ID(square); SET_DEFAULT(1.0f);

		OP_BLOCK(data, value)
		{
			for (auto ch : data)
				hmath::vmul(data.toChannelData(ch), data.toChannelData(ch));
		}

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s *= s;
		}
	};

	struct sqrt
	{
		SET_ID(sqrt); SET_DEFAULT(1.0f);

		OP_BLOCK2SINGLE(data, unused);

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s = sqrtf(s);
		}
	};

	struct pow
	{
		SET_ID(pow); SET_DEFAULT(1.0f);

		OP_BLOCK2SINGLE(data, unused);

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s = powf(s, value);
		}
	};

	struct abs
	{
		SET_ID(abs); SET_DEFAULT(0.0f);

		OP_BLOCK(data, unused)
		{
			for (auto& ch : data)
				hmath::vabs(data.toChannelData(ch));
		}

		OP_SINGLE(data, value)
		{
			for (auto& s : data)
				s = hmath::abs(s);
		}
	};
}



template <class OpType, int V> class OpNode : public HiseDspBase
{
public:

	using OperationType = OpType;

	enum class Parameters
	{
		Value
	};

	DEFINE_PARAMETERS
	{
		DEF_PARAMETER(Value, OpNode);
	}

	constexpr static int NumVoices = V;

	SET_HISE_POLY_NODE_ID(OpType::getId());
	SN_GET_SELF_AS_OBJECT(OpNode);
	HISE_EMPTY_HANDLE_EVENT;

	bool handleModulation(double&) noexcept;;
	
	template <typename PD> void process(PD& d)
	{
		OpType::op<PD>(d, value.get());
	}

	template <typename FD> void processFrame(FD& d)
	{
		OpType::opSingle(d, value.get());
	}

	void reset() noexcept;
	void prepare(PrepareSpecs ps);
	void createParameters(ParameterDataList& data) override;
	void setValue(double newValue);

#if 0
	static snex::Types::DefaultFunctionClass createSnexFunctions(const snex::Types::SnexTypeConstructData& cd);
#endif

	PolyData<float, NumVoices> value = OpType::defaultValue;
};

#define DEFINE_OP_NODE_IMPL(opName) template class OpNode<Operations::opName, 1>; \
template class OpNode<Operations::opName, NUM_POLYPHONIC_VOICES>;

#define DEFINE_MONO_OP_NODE_IMPL(opName) template class OpNode<Operations::opName, 1>; \

#define DEFINE_MONO_OP_NODE(monoName) extern template class OpNode<Operations::monoName, 1>; \
using monoName = OpNode<Operations::monoName, 1>;

#define DEFINE_OP_NODE(monoName, polyName) extern template class OpNode<Operations::monoName, 1>; \
using monoName = OpNode<Operations::monoName, 1>; \
extern template class OpNode<Operations::monoName, NUM_POLYPHONIC_VOICES>; \
using polyName = OpNode<Operations::monoName, NUM_POLYPHONIC_VOICES>;

DEFINE_OP_NODE(mul, mul_poly);
DEFINE_OP_NODE(add, add_poly);
DEFINE_OP_NODE(sub, sub_poly);
DEFINE_OP_NODE(div, div_poly);
DEFINE_OP_NODE(tanh, tanh_poly);
DEFINE_OP_NODE(clip, clip_poly);
DEFINE_MONO_OP_NODE(sin);
DEFINE_MONO_OP_NODE(pi);
DEFINE_MONO_OP_NODE(sig2mod);
DEFINE_MONO_OP_NODE(abs);
DEFINE_MONO_OP_NODE(clear);
DEFINE_MONO_OP_NODE(square);
DEFINE_MONO_OP_NODE(sqrt);
DEFINE_MONO_OP_NODE(pow);

}


}