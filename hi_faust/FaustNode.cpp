#include <faust/dsp/llvm-dsp.h>
namespace scriptnode {
namespace faust {
    // wrapper struct for faust types to avoid name-clash
    struct faust_wrapper {

	faust_wrapper():
	    sampleRate(0),
	    jitFactory(nullptr),
	    jitDsp(nullptr)
	{ }
	
	std::string code;
	std::string errorMessage;
	int jitOptimize = 0; // -1 is maximum optimization
	int sampleRate;
	llvm_dsp_factory* jitFactory;
	dsp *jitDsp;

	void recompile() {
	    // cleanup old code and factories
	    if (jitDsp != nullptr) {
		delete jitDsp;
	    }
	    if (jitFactory != nullptr) {
		deleteDSPFactory(jitFactory);
	    }

	    int llvm_argc = 0;
	    const char* llvm_argv[] = {nullptr};

	    jitFactory = createDSPFactoryFromString("faust", code, llvm_argc, llvm_argv, "",
						      errorMessage, jitOptimize);
	    if (jitFactory == nullptr) {
		// TODO error indication
		return;
	    }

	    jitDsp = jitFactory->createDSPInstance();
	    if (jitDsp == nullptr) {
		// TODO error indication
		return;
	    }
	}
    };
    // faust_node::faust_node(DspNetwork* n, ValueTree v) :
    // 	NodeBase(n, v, 0) { }
    faust_node::faust_node(DspNetwork* n, ValueTree v) :
	NodeBase(n, v, 0),
	faust(new faust_wrapper)
	{
	    // dummy code for now:
	    faust->code = "import(\"stdfaust.lib\"); d(x) = x + X@10000; process = d";
	}

    void faust_node::initialise(NodeBase* n)
    { }

    void faust_node::prepare(PrepareSpecs specs)
    {
      lastSpecs = specs;
      // recompile if sample rate changed
      int newSampleRate = (int)specs.sampleRate;
      if (newSampleRate != faust->sampleRate) {
	faust->sampleRate = newSampleRate;
	// recompile
	faust->recompile();
      }
    }

    void faust_node::reset()
    { }

    void faust_node::process(ProcessDataDyn& data)
    { }

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
}
}
