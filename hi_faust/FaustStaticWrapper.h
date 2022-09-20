#ifndef __FAUST_STATIC_WRAPPER_H
#define __FAUST_STATIC_WRAPPER_H

namespace scriptnode {
namespace faust{

template <int NV, class FaustClass, class MC> struct faust_static_wrapper: public data::base, public faust_base_wrapper
{
	// Metadata Definitions ------------------------------------------------------
	
	SNEX_NODE(faust_static_wrapper);

	using MetadataClass = MC;
	
	// set to true if you want this node to have a modulation dragger
	static constexpr bool isModNode() { return false; };
	static constexpr bool isPolyphonic() { return NV > 1; };
	// set to true if your node produces a tail
	static constexpr bool hasTail() { return false; };
	// Undefine this method if you want a dynamic channel count
	static constexpr int getFixChannelAmount() { return 2; };
	
	// Define the amount and types of external data slots you want to use
	static constexpr int NumTables = 0;
	static constexpr int NumSliderPacks = 0;
	static constexpr int NumAudioFiles = 0;
	static constexpr int NumFilters = 0;
	static constexpr int NumDisplayBuffers = 0;

	FaustClass faust_obj;

	faust_static_wrapper():
		faust_base_wrapper(&faust_obj)
	{
		faust_obj.buildUserInterface(&ui);
	}
	
	// Scriptnode Callbacks ------------------------------------------------------
	
	
	void handleHiseEvent(HiseEvent& e)
	{
		
	}
	
	template <typename T> void process(T& data)
	{
		faust_base_wrapper::process(data.template as<ProcessDataDyn>());
	}
	
	template <typename T> void processFrame(T& data)
	{
		
	}
	
	int handleModulation(double& value)
	{
		
		return 0;
		
	}
	
	void setExternalData(const ExternalData& data, int index)
	{
		
	}
	// Parameter Functions -------------------------------------------------------
	
	template <int P> void setParameter(double v)
	{
		jassert(P < ui.parameters.size());
		*(ui.parameters[P]->zone) = (float)v;
	}
	
	void createParameters(ParameterDataList& data)
	{
		int i = 0;
		for (const auto& p : ui.parameters)
		{
			auto pd = p->toParameterData();
			pd.callback.referTo((void*)p->zone, [](void* obj, double newValue) { *((float*)obj) = (float)newValue; });
			pd.info.index = i++;
			data.add(std::move(pd));
		}
	}
};
}
}

#endif // __FAUST_STATIC_WRAPPER_H

#if 0
// generated code
using Meta = faust::Meta;
using UI = faust::UI;
#include "src/stereo-delay.cpp"
namespace project {
struct StereoDelayMetaData {
		SN_NODE_ID("faust_mockup");
};
template <int NV>
using faust_mockup = scriptnode::faust::faust_static_wrapper<1, _stereo_delay, StereoDelayMetaData>;
}

#endif // 0
