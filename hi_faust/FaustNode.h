#pragma once

namespace scriptnode {
namespace faust {

struct faust_wrapper;

struct faust_node: public scriptnode::NodeBase {
    SET_HISE_NODE_ID("faust");
    faust_node(DspNetwork* n, ValueTree v);
    void initialise(NodeBase* n);
    virtual void prepare(PrepareSpecs specs) override;
    virtual void reset() override;
    virtual void process(ProcessDataDyn& data) override;
    virtual void processFrame(FrameType& data) override;
    static NodeBase* createNode(DspNetwork* n, ValueTree v);
    File getFaustRootFile(NodeBase* n);

private:
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

