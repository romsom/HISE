#pragma once

namespace scriptnode
{
using namespace juce;
using namespace hise;

namespace etxzat
{
struct etxzat_node : public HiseDspBase
{
	SET_HISE_NODE_ID("etxzat_node");
	SET_HISE_NODE_EXTRA_HEIGHT(0);
	GET_SELF_AS_OBJECT(etxzat_node);
	SET_HISE_NODE_IS_MODULATION_SOURCE(false);

	etxzat_node() {};

	void prepare(PrepareSpecs ps);

	bool handleModulation(double&) noexcept { return false; };
	void reset() noexcept;

	void process(ProcessData& d) noexcept;
	void processSingle(float* numFrames, int numChannels) noexcept;
	void setDelayTimeMilliseconds(double newValue);
	void setFadeTimeMilliseconds(double newValue);
	void createParameters(Array<ParameterData>& data) override;

private:
	void setGain(float new_gain) { gain = new_gain; }
	float gain;
};
}
}
