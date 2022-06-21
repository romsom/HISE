namespace scriptnode {
namespace faust {

struct faust_node: public scriptnode::NodeBase {
    SET_HISE_NODE_ID("faust");
    // using WrappedObjectType = NodeBase; // What does this type do?
    // What does this struct do?
    // struct Comp : public NodeComponent {
    // 	Comp(faust_node* b) :
    // 	    NodeComponent(b) { }
    // };

    // Why is the constructor of this struct called with no parameters?
    faust_node(DspNetwork* n, ValueTree v) :
	NodeBase(n, v, 0) { }
    void initialise(NodeBase* n) { }
    void process(ProcessDataDyn& data) override { }
    void processFrame(FrameType& data) override { } // Why does the compiler complain about override here, but not in SpecNode?
};
}
}
