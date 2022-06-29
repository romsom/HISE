#pragma once

// Faust dsp class
struct dsp;
struct llvm_dsp_factory;

namespace scriptnode {
namespace faust {


struct faust_node: public scriptnode::NodeBase {
    SET_HISE_NODE_ID("faust");
    faust_node(DspNetwork* n, ValueTree v) :
	NodeBase(n, v, 0),
	sampleRate(0),
	faustFactory(nullptr),
	faustDsp(nullptr)
	{
	    // dummy code for now:
	    faustCode = "import(\"stdfaust.lib\"); d(x) = x + X@10000; process = d";
	}
    // faust_node(DspNetwork* n, ValueTree v);
    void initialise(NodeBase* n);
    virtual void prepare(PrepareSpecs specs) override;
    virtual void reset() override;
    virtual void process(ProcessDataDyn& data) override;
    virtual void processFrame(FrameType& data) override;
    static NodeBase* createNode(DspNetwork* n, ValueTree v);
    File getFaustRootFile(NodeBase* n);

private:
    void recompileFaustCode();
    std::string faustCode;
    std::string faustErrorMessage;
    int faustJitOptimize = 0; // -1 is maximum optimization
    int sampleRate;
    llvm_dsp_factory* faustFactory;
    dsp *faustDsp;
};
}
}

