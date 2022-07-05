#include <faust_wrap/dsp/llvm-dsp.h>

namespace scriptnode {
namespace faust {
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
	    return true;
	}

	void init() {
	    if (jitDsp)
		jitDsp->init(sampleRate);
	}
    };

    // faust_node::faust_node(DspNetwork* n, ValueTree v) :
    // 	NodeBase(n, v, 0) { }
    faust_node::faust_node(DspNetwork* n, ValueTree v) :
	NodeBase(n, v, 0),
	faust(new faust_wrapper)
	{
	    // dummy code for now:
	    faust->code = "import(\"stdfaust.lib\"); d(x) = x + x@10000; process = d,d;";
	    // setup dsp
	    bool success = faust->setup();
	    std::cout << "Faust initialization: " << (success ? "success" : "failed") << std::endl;
	    // TODO: error handling

            // we can't init yet, because we don't know the sample rate
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
	    // recompile
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

}
}
