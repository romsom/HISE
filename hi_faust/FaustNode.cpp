#define FAUSTFLOAT float

#include <faust_wrap/dsp/llvm-dsp.h>
#include <faust_wrap/gui/UI.h>

namespace scriptnode {
namespace faust {
struct faust_ui : public ::faust::UI {
    enum ControlType {
        NONE = 0,
        BUTTON,
        CHECK_BUTTON,
        VERTICAL_SLIDER,
        HORIZONTAL_SLIDER,
        NUM_ENTRY,
        HORIZONTAL_BARGRAPH,
        VERTICAL_BARGRAPH,
        // SOUND_FILE, // Handle Soundfile separately
        MIDI,
        OTHER=0xffff,
    };

    struct Parameter {
        ControlType type;
        String label;
        float* zone;
        float init;
        float min;
        float max;
        float step;

        Parameter(ControlType type,
                  String label,
                  float* zone,
                  float init,
                  float min,
                  float max,
                  float step) :
            type(type),
            label(label),
            zone(zone),
            init(init),
            min(min),
            max(max),
            step(step) {}

    };

    faust_ui() { }

    std::vector<std::shared_ptr<Parameter>> parameters;

    std::optional<std::shared_ptr<Parameter>> getParameterByLabel(String label)
    {
        for (auto p : parameters)
        {
            if (p->label == label)
                return p;
        }
        return {};
    }

    std::vector<String> getParameterLabels()
    {
        std::vector<String> res;
        res.reserve(parameters.size());

        for (auto p : parameters)
        {
            res.push_back(p->label);
        }

        return res;
    }

    float* getZone(String label) {
        for (auto p : parameters)
        {
            if (p->label == label)
                return p->zone;
        }
        return nullptr;
    }


    // -- metadata declarations

    virtual void declare(float* zone, const char* key, const char* val) override
    {
        // TODO
    }

    // Faust UI implementation

    virtual void openTabBox(const char* label) override
    {

    }
    virtual void openHorizontalBox(const char* label) override
    {

    }
    virtual void openVerticalBox(const char* label) override
    {

    }
    virtual void closeBox() override { }

    // -- active widgets

    virtual void addButton(const char* label, float* zone) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::BUTTON,
                                                         String(label),
                                                         zone,
                                                         0.f,
                                                         0.f,
                                                         1.f,
                                                         1.f));
    }
    virtual void addCheckButton(const char* label, float* zone) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::CHECK_BUTTON,
                                                         String(label),
                                                         zone,
                                                         0.f,
                                                         0.f,
                                                         1.f,
                                                         1.f));

    }
    virtual void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::VERTICAL_SLIDER,
                                                         String(label),
                                                         zone,
                                                         init,
                                                         min,
                                                         max,
                                                         step));
    }
    virtual void addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::HORIZONTAL_SLIDER,
                                                         String(label),
                                                         zone,
                                                         init,
                                                         min,
                                                         max,
                                                         step));
    }
    virtual void addNumEntry(const char* label, float* zone, float init, float min, float max, float step) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::NUM_ENTRY,
                                                         String(label),
                                                         zone,
                                                         init,
                                                         min,
                                                         max,
                                                         step));
    }

    // -- passive widgets

    virtual void addHorizontalBargraph(const char* label, float* zone, float min, float max) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::HORIZONTAL_BARGRAPH,
                                                         String(label),
                                                         zone,
                                                         0.f,
                                                         min,
                                                         max,
                                                         1.f));
    }
    virtual void addVerticalBargraph(const char* label, float* zone, float min, float max) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::VERTICAL_BARGRAPH,
                                                         String(label),
                                                         zone,
                                                         0.f,
                                                         min,
                                                         max,
                                                         1.f));
    }

    // -- soundfiles -- TODO

    virtual void addSoundfile(const char* label, const char* filename, ::faust::Soundfile** sf_zone) { }

};

// wrapper struct for faust types to avoid name-clash
struct faust_wrapper {

    faust_wrapper():
        sampleRate(0),
        jitFactory(nullptr),
        jitDsp(nullptr)
    { }

    ~faust_wrapper()
    {
        if(jitDsp)
            delete jitDsp;
        if (jitFactory)
            ::faust::deleteDSPFactory(jitFactory);
    }

    std::string code;
    std::string errorMessage;
    int jitOptimize = 0; // -1 is maximum optimization
    int sampleRate;
    ::faust::llvm_dsp_factory* jitFactory;
    ::faust::dsp *jitDsp;
    faust_ui ui;

    bool setup() {
        // cleanup old code and factories
        if (jitDsp != nullptr) {
            delete jitDsp;
        }
        if (jitFactory != nullptr) {
            ::faust::deleteDSPFactory(jitFactory);
        }

        int llvm_argc = 0;
        const char* llvm_argv[] = {nullptr};

        jitFactory = ::faust::createDSPFactoryFromString("faust", code, llvm_argc, llvm_argv, "",
                                                         errorMessage, jitOptimize);
        if (jitFactory == nullptr) {
            // TODO error indication
            std::cout << "Faust jit compilation failed:" << std::endl
                      << errorMessage << std::endl;
            return false;
        }
        std::cout << "Faust jit compilation successful" << std::endl;
        jitDsp = jitFactory->createDSPInstance();
        if (jitDsp == nullptr) {
            std::cout << "Faust DSP instantiation" << std::endl;
            return false;
        }

        std::cout << "Faust DSP instantiation successful" << std::endl;

        jitDsp->buildUserInterface(&ui);


        DBG("Faust parameters:");
        for (auto p : ui.getParameterLabels()) {
            DBG(p);
        }

        return true;
    }

    void init() {
        if (jitDsp)
            jitDsp->init(sampleRate);
    }
};

// faust_node::faust_node(DspNetwork* n, ValueTree v) :
//      NodeBase(n, v, 0) { }
faust_node::faust_node(DspNetwork* n, ValueTree v) :
    WrapperNode(n, v),
    faust(new faust_wrapper)
{
    // dummy code for now:
    faust->code =
        "import(\"math.lib\");\n"
        "import(\"filter.lib\");\n"
        "\n"
        "DELAY_MS_MAX = 25;\n"
        "SR_MAX = 192000.0;\n"
        "SR_ = min(SR, SR_MAX);\n"
        "\n"
        "ipt = hslider(\"smooth_time\", 0.05, 0, 0.1, 0.001); // s\n"
        "ip = smooth(tau2pole(ipt));\n"
        "// : is aggregate, read as signal flow from left to right\n"
        "// smooth to remove glitches, when moving the slider, then limit, so buffer size can be inferred\n"
        "d = hslider(\"depth\", 5, 0, DELAY_MS_MAX, 0.01) : ip : min(DELAY_MS_MAX) : max(0); // ms\n"
        "a = hslider(\"amount\", 0.35, 0, 1, 0.01) : ip : min(1) : max(0); // relative amount\n"
        "f = hslider(\"feedback\", 0.75, 0, 1, 0.001) : ip; // amount of feedback\n"
        "s = hslider(\"speed\", 7, 0.1, 25, 0.01) : ip : max(0.1); // hz\n"
        "l = hslider(\"output level\", 0.5, 0, 1, 0.01) : ip;\n"
        "\n"
        "fixed_fdel(n) = \\(x).((1-a) * x@nInt + a * x@(nInt + 1))\n"
        "with {\n"
        "     nInt = int(n);\n"
        "     a = n - nInt;\n"
        "};\n"
        "\n"
        "lfo(wf, p) = p * float(arg) / nSamples : wf\n"
        "with {\n"
        "    nSamples = int(SR / s);\n"
        "    arg = +(1) ~ \\(x).( x * (x % nSamples != 0)) ;\n"
        "};\n"
        "\n"
        "// discard util signal here\n"
        "c(x) = cal(x) : \\(x,y).(x * l)\n"
        "with {\n"
        "     cal(x) = (+(x), +(a * x)) ~ (sd : mux)\n"
        "     with {\n"
        "         // single delay stage, customize\n"
        "     // a instead of d works quite well for simple controls\n"
        "     // TODO: try to adjust depth according to speed, so depth defines the derivative of lfo\n"
        "     // note: /s is too much, also prohibits to set s=0\n"
        "         sd(x,y) = y : fixed_fdel((lfo(sin, 2*PI) + 1) / 2 * a * SR_ / 1000);\n"
        "     // output signal, feedback signal\n"
        "             mux = _ <: _, f*_;\n"
        "    };\n"
        "};\n"
        "\n"
        "process = c, c;\n"

        "";
    // setup dsp
    bool success = faust->setup();
    std::cout << "Faust initialization: " << (success ? "success" : "failed") << std::endl;
    // TODO: error handling
    if (!success)
    {
        auto p = dynamic_cast<Processor*>(getScriptProcessor());

        debugError(p, "FaustError");
    }

    parameterListener.setCallback(getParameterTree(),
                                  valuetree::AsyncMode::Synchronously, BIND_MEMBER_FUNCTION_2(parameterUpdated));


    setupParameters();
    // we can't init yet, because we don't know the sample rate
}

void faust_node::setupParameters()
{
    // setup parameters from faust code
    for (auto p : faust->ui.parameters)
    {
        switch (p->type) {
        case faust_ui::ControlType::VERTICAL_SLIDER:
        case faust_ui::ControlType::HORIZONTAL_SLIDER:
        {
            DBG("adding parameter " << p->label);
            parameter::data pd(p->label, {(double)(p->min), (double)(p->max)});
            pd.setDefaultValue((double)(p->init));
            addNewParameter(pd);
        }
        }
    }
    DBG("Num parameters in NodeBase: " << getNumParameters());
}

static void updateFaustZone(void* obj, double value)
{
    *static_cast<float*>(obj) = (float)value;
}

// ParameterTree listener callback: This function is called when the ParameterTree changes
void faust_node::parameterUpdated(ValueTree child, bool wasAdded)
{
    if (wasAdded)
    {
        String parameterLabel = child.getProperty(PropertyIds::ID);
        auto faustParameter = faust->ui.getParameterByLabel(parameterLabel).value_or(nullptr);

        // proceed only if this parameter was defined in faust (check by label)
        if (!faustParameter)
            return;

        float* zonePointer = faustParameter->zone;

        // setup dynamic parameter
        auto dp = new scriptnode::parameter::dynamic();
        // install callback and zone pointer into dynamic parameter
        dp->referTo(zonePointer, updateFaustZone);

        // create dynamic_base from dynamic parameter to attach to the new Parameter
        auto dyn_base = new scriptnode::parameter::dynamic_base(*dp);

        // create and setup parameter
        auto p = new scriptnode::Parameter(this, child);
        p->setDynamicParameter(dyn_base);

        NodeBase::addParameter(p);
    }
    else
    {
        auto index = child.getParent().indexOf(child);
        NodeBase::removeParameter(index);
    }
}

void faust_node::initialise(NodeBase* n)
{
}

void faust_node::prepare(PrepareSpecs specs)
{
    lastSpecs = specs;
    // recompile if sample rate changed
    int newSampleRate = (int)specs.sampleRate;
    if (newSampleRate != faust->sampleRate) {
        faust->sampleRate = newSampleRate;
        // init samplerate
        faust->init();
    }

    if (_nChannels != specs.numChannels || _nFramesMax != specs.blockSize) {
        std::cout << "Faust: Resizing buffers" << std::endl;
        _nChannels = specs.numChannels;
        _nFramesMax = specs.blockSize;
        resizeBuffer();
    }
}

void faust_node::reset()
{
    if (faust->jitDsp) {
        faust->jitDsp->instanceClear();
    }
}

void faust_node::process(ProcessDataDyn& data)
{
    if (isBypassed()) return;

    if (faust->jitDsp) {
        // TODO: stable and sane sample format matching
        int n_faust_inputs = faust->jitDsp->getNumInputs();
        int n_faust_outputs = faust->jitDsp->getNumOutputs();
        int n_hise_channels = data.getNumChannels();

        if (n_faust_inputs == n_hise_channels && n_faust_outputs == n_hise_channels) {
            int nFrames = data.getNumSamples();
            float** channel_data = data.getRawDataPointers();
            // copy input data, because even with -inpl not all faust generated code can handle
            // in-place processing
            bufferChannelsData(channel_data, n_hise_channels, nFrames);
            faust->jitDsp->compute(nFrames, getRawInputChannelPointers(), channel_data);
        } else {
            // TODO error indication
        }
    } else {
        // std::cout << "Faust: dsp was not initialized" << std::endl;
    }
}

void faust_node::processFrame(FrameType& data)
{ }

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

void faust_node::resizeBuffer()
{
    inputBuffer.resize(_nChannels * _nFramesMax);
    // setup new pointers
    inputChannelPointers.resize(_nChannels);
    inputChannelPointers.clear();
    for (int i=0; i<inputBuffer.size(); i+=_nFramesMax) {
        inputChannelPointers.push_back(&inputBuffer[i]);
    }
}

void faust_node::bufferChannelsData(float** channels, int nChannels, int nFrames)
{
    assert(nChannels == _nChannels);
    assert(nFrames <= _nFramesMax);

    for (int i=0; i<nChannels; i++) {
        memcpy(inputChannelPointers[i], channels[i], nFrames * sizeof(float));
    }
}

faust_node::FaustMenuBar::FaustMenuBar(faust_node *n) :
    addButton("add", this, factory),
    editButton("faust", this, factory),
    node(n)
{
    setLookAndFeel(&claf);
    setSize(200, 24);
    addAndMakeVisible(sourceSelector);
    addAndMakeVisible(addButton);
    addAndMakeVisible(editButton);
}

void faust_node::FaustMenuBar::resized()
{
    auto b = getLocalBounds().reduced(0, 1);
    auto h = getHeight();

    addButton.setBounds(b.removeFromLeft(h-4));
    sourceSelector.setBounds(b.removeFromLeft(100));
    b.removeFromLeft(3);
    editButton.setBounds(getLocalBounds().removeFromRight(80).reduced(2));

    b.removeFromLeft(10);
}

void faust_node::FaustMenuBar::buttonClicked(Button* b)
{
    // TODO
}
void faust_node::FaustMenuBar::comboBoxChanged (ComboBox *comboBoxThatHasChanged)
{
    // TODO
}

juce::Path faust_node::FaustMenuBar::Factory::createPath(const String& url) const
{
    if (url == "snex")
    {
        snex::ui::SnexPathFactory f;
        return f.createPath(url);
    }

    Path p;

    LOAD_PATH_IF_URL("new", ColumnIcons::threeDots);
    LOAD_PATH_IF_URL("edit", ColumnIcons::openWorkspaceIcon);
    LOAD_PATH_IF_URL("compile", EditorIcons::compileIcon);
    LOAD_PATH_IF_URL("reset", EditorIcons::swapIcon);
    LOAD_PATH_IF_URL("add", ColumnIcons::threeDots);

    return p;
}

void faust_node::addNewParameter(parameter::data p)
{
    if (auto existing = getParameterFromName(p.info.getId()))
        return;

    auto newTree = p.createValueTree();
    getParameterTree().addChild(newTree, -1, getUndoManager());
}

}
}
