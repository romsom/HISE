#pragma once

namespace scriptnode {
namespace faust {

struct faust_node: public scriptnode::NodeBase {
    SET_HISE_NODE_ID("faust");
    faust_node(DspNetwork* n, ValueTree v) :
	NodeBase(n, v, 0) { }
    void initialise(NodeBase* n) { }
    virtual void prepare(PrepareSpecs specs) override { }
    virtual void reset() override { }
    virtual void process(ProcessDataDyn& data) override { }
    virtual void processFrame(FrameType& data) override { }
    static NodeBase* createNode(DspNetwork* n, ValueTree v)
	{
	    return new faust_node(n, v);
	}
    File getFaustRootFile(NodeBase* n)
	{
	    auto mc = n->getScriptProcessor()->getMainController_();
	    auto dspRoot = mc->getCurrentFileHandler().getSubDirectory(FileHandlerBase::DspNetworks);
	    return dspRoot.getChildFile("CodeLibrary/faust");
	}
};
}
}

