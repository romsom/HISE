namespace scriptnode {
namespace faust {
    // faust_node::faust_node(DspNetwork* n, ValueTree v) :
    // 	NodeBase(n, v, 0) { }
    void faust_node::initialise(NodeBase* n) { }
    void faust_node::prepare(PrepareSpecs specs) { }
    void faust_node::reset() { }
    void faust_node::process(ProcessDataDyn& data) { }
    void faust_node::processFrame(FrameType& data) { }
    NodeBase* faust_node::createNode(DspNetwork* n, ValueTree v)
	{
	    return new faust_node(n, v);
	}
    File faust_node::getFaustRootFile(NodeBase* n)
	{
	    auto mc = n->getScriptProcessor()->getMainController_();
	    auto dspRoot = mc->getCurrentFileHandler().getSubDirectory(FileHandlerBase::DspNetworks);
	    return dspRoot.getChildFile("CodeLibrary/faust");
	}
}
}
