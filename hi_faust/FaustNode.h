#pragma once

namespace scriptnode {
namespace faust {

struct faust_wrapper;

struct faust_node: public scriptnode::NodeBase {
    SET_HISE_NODE_ID("faust");

    struct FaustMenuBar : public Component,
			 public ButtonListener,
			 public ComboBox::Listener

    {

	FaustMenuBar(faust_node *n);
	HiseShapeButton addButton;
	HiseShapeButton editButton;
	struct Factory : public PathFactory
	{
	    Path createPath(const String& p) const override;
	    String getId() const override { return {}; }
	} factory;

	WeakReference<faust_node> node;
    };

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
    //FaustMenuBar menuBar;
    void resizeBuffer();
    float** getRawInputChannelPointers() {
	return &inputChannelPointers[0];
    }
    void bufferChannelsData(float** channels, int nChannels, int nFrames);
};
}
}

