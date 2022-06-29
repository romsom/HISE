#include <faust/dsp/llvm-dsp.h>
namespace scriptnode {
namespace faust {
    // faust_node::faust_node(DspNetwork* n, ValueTree v) :
    // 	NodeBase(n, v, 0) { }

    void faust_node::initialise(NodeBase* n)
    { }

    void faust_node::prepare(PrepareSpecs specs)
    {
      lastSpecs = specs;
      // recompile if sample rate changed
      int newSampleRate = (int)specs.sampleRate;
      if (newSampleRate != sampleRate) {
	sampleRate = newSampleRate;
	// recompile
	
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

    void faust_node::recompileFaustCode() {
	// cleanup old code and factories
	if (faustDsp != nullptr) {
	    delete faustDsp;
	}
	if (faustFactory != nullptr) {
	    deleteDSPFactory(faustFactory);
	}

	int llvm_argc = 0;
	const char* llvm_argv[] = {nullptr};

	faustFactory = createDSPFactoryFromString("faust", faustCode, llvm_argc, llvm_argv, "",
						  faustErrorMessage, faustJitOptimize);
	if (faustFactory == nullptr) {
	    // TODO error indication
	    return;
	}

	faustDsp = faustFactory->createDSPInstance();
	if (faustDsp == nullptr) {
	    // TODO error indication
	    return;
	}
    }
}
}
