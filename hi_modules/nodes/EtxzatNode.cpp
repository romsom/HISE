namespace scriptnode
{
using namespace juce;
using namespace hise;

namespace etxzat
{
void etxzat_node::prepare(PrepareSpecs ps) {}

void etxzat_node::reset() noexcept {}

void etxzat_node::createParameters(Array<ParameterData>& data)
{
	{
		ParameterData p("Gain");
		p.db = std::bind(&etxzat_node::setGain, this, std::placeholders::_1);
		p.range = { -2.0, 2.0, 0.01 };
		p.defaultValue = 1.0;
		data.add(std::move(p));
	}
}

void etxzat_node::process(ProcessData& d) noexcept
{
	for (unsigned int i=0; i<d.numChannels; i++)
	{
		for (unsigned int j=0; j<d.size; j++)
		{
			d.data[i][j] *= gain;
		}
	}
}

void etxzat_node::processSingle(float* numFrames, int numChannels) noexcept
{
	for (unsigned int i=0; i<numChannels; i++)
	{
		numFrames[i] *= gain;
	}
}

}
}
