#pragma once

namespace scriptnode {
namespace faust {

struct faust_wrapper;

struct faust_node: public scriptnode::WrapperNode
{
    SET_HISE_NODE_ID("faust");
    JUCE_DECLARE_WEAK_REFERENCEABLE(faust_node);

    struct FaustMenuBar : public Component,
			  public ButtonListener,
			  public ComboBox::Listener

    {

	FaustMenuBar(faust_node *n);
	struct Factory : public PathFactory
	{
	    Path createPath(const String& p) const override;
	    String getId() const override { return {}; }
	} factory;

	ComboBox sourceSelector;
	HiseShapeButton addButton;
	HiseShapeButton editButton;

	virtual void buttonClicked(Button* b) override;
	virtual void comboBoxChanged (ComboBox *comboBoxThatHasChanged) override;
	virtual void resized() override;

	WeakReference<faust_node> node;
    };

    faust_node(DspNetwork* n, ValueTree v);
    void initialise(NodeBase* n);
    virtual void prepare(PrepareSpecs specs) override;
    virtual void reset() override;
    virtual void process(ProcessDataDyn& data) override;
    virtual void processFrame(FrameType& data) override;
    static NodeBase* createNode(DspNetwork* n, ValueTree v);
    virtual scriptnode::NodeComponent* createComponent() override;
    File getFaustRootFile(NodeBase* n);

    void addNewParameter(parameter::data p);

    virtual void* getObjectPtr() override { return nullptr; }

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

