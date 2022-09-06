#pragma once

namespace scriptnode {
namespace faust {

struct faust_wrapper;

struct faust_base_node_base: public scriptnode::WrapperNode
{
    SN_NODE_ID("faust");
    JUCE_DECLARE_WEAK_REFERENCEABLE(faust_base_node_base);
    virtual Identifier getTypeId() const { RETURN_STATIC_IDENTIFIER("faust"); }

    faust_base_node_base(DspNetwork* n, ValueTree v);
    void initialise(NodeBase* n);
    virtual void prepare(PrepareSpecs specs) override;
    virtual void reset() override;
    virtual void process(ProcessDataDyn& data) override;
    virtual void processFrame(FrameType& data) override;
    // createNode() will have to be supplied by every derived class
    static NodeBase* createNode(DspNetwork* n, ValueTree v) { return new faust_base_node_base(n, v); }
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
    std::unique_ptr<faust_wrapper> faust;
    // NodePropertyT<String> classId;
    // void updateClassId(Identifier, var newValue);
};
}
}

