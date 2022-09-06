#pragma once

namespace scriptnode {
namespace faust {

struct faust_jit_wrapper;

struct faust_jit_node: public faust_base_node
{
    SN_NODE_ID("faust_jit_node");
    JUCE_DECLARE_WEAK_REFERENCEABLE(faust_jit_node);
	virtual Identifier getTypeId() const { RETURN_STATIC_IDENTIFIER("faust_jit_node"); }

    faust_jit_node(DspNetwork* n, ValueTree v);
    void initialise(NodeBase* n);
    virtual void prepare(PrepareSpecs specs) override;
    virtual void reset() override;
    virtual void process(ProcessDataDyn& data) override;
    virtual void processFrame(FrameType& data) override;
    static NodeBase* createNode(DspNetwork* n, ValueTree v);
    File getFaustRootFile();
    File getFaustFile(String basename);

    void parameterUpdated(ValueTree child, bool wasAdded);

    void addNewParameter(parameter::data p);

    // provide correct pointer to createExtraComponent()
    virtual void* getObjectPtr() override { return (void*)this; }

    String getClassId();
    void loadSource();
    void setClass(const String& newClassId);

    valuetree::ChildListener parameterListener;

	StringArray getAvailableClassIds();

private:
    void setupParameters();
    void resetParameters();
    // void recompileFaustCode();
    std::unique_ptr<faust_wrapper> faust;
    NodePropertyT<String> classId;
    void updateClassId(Identifier, var newValue);
};
}
}

