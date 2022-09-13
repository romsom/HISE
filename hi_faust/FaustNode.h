#ifndef __FAUST_NODE
#define __FAUST_NODE

namespace scriptnode {
namespace faust {

struct faust_base_wrapper;

struct faust_base_node: public scriptnode::WrapperNode
{
    SN_NODE_ID("faust");
    JUCE_DECLARE_WEAK_REFERENCEABLE(faust_base_node);
    virtual Identifier getTypeId() const { RETURN_STATIC_IDENTIFIER("faust"); }

	faust_base_node(DspNetwork* n, ValueTree v, faust_base_wrapper* faustPtr);
    void initialise(NodeBase* n);
    virtual void prepare(PrepareSpecs specs) override;
    virtual void reset() override;
    virtual void process(ProcessDataDyn& data) override;
    virtual void processFrame(FrameType& data) override;
    // createNode() will have to be supplied by every derived class
	static NodeBase* createNode(DspNetwork* n, ValueTree v);
    // File getFaustRootFile();
    // File getFaustFile(String basename);

    // Pure virtual to set/get the class in faust_jit_node and
    // only get in faust_node<T>, here because of UI code
    virtual String getClassId();
    virtual StringArray getAvailableClassIds();
    virtual void setClass(const String& newClassId);
    // void loadSource();

    // Parameter methods
    void parameterUpdated(ValueTree child, bool wasAdded);
    void addNewParameter(parameter::data p);
    valuetree::ChildListener parameterListener;

    // provide correct pointer to createExtraComponent()
    virtual void* getObjectPtr() override { return (void*)this; }


private:
    void setupParameters();
    void resetParameters();
    // void recompileFaustCode();
    std::unique_ptr<faust_base_wrapper> faust;
    // NodePropertyT<String> classId;
    // void updateClassId(Identifier, var newValue);
};

template <class FaustDsp>
struct faust_node: public faust_base_node {
	faust_node(DspNetwork* n, ValueTree v):
		faust_base_node(n, v, (faust_base_wrapper*)(new faust_wrapper<FaustDsp>))
	{ }
    static NodeBase* createNode(DspNetwork* n, ValueTree v) { return new faust_node(n, v); }
};
}
}

#endif // __FAUST_NODE
