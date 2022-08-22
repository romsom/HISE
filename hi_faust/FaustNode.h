#pragma once

namespace scriptnode {
namespace faust {

struct faust_wrapper;

struct faust_node: public scriptnode::WrapperNode
{
    SET_HISE_NODE_ID("faust");
    JUCE_DECLARE_WEAK_REFERENCEABLE(faust_node);


    faust_node(DspNetwork* n, ValueTree v);
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

    String getSourceId();
    void loadSource(String& newSourceId);

    valuetree::ChildListener parameterListener;

private:
    void setupParameters();
    void recompileFaustCode();
    std::unique_ptr<faust_wrapper> faust;
    int _nChannels;
    int _nFramesMax;
    std::vector<float> inputBuffer;
    std::vector<float*> inputChannelPointers;
    void resizeBuffer();
    float** getRawInputChannelPointers() {
        return &inputChannelPointers[0];
    }
    void bufferChannelsData(float** channels, int nChannels, int nFrames);
};
}
}

