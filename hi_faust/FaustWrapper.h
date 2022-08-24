#ifndef __FAUST_WRAPPER_H

namespace scriptnode {
namespace faust {

// wrapper struct for faust types to avoid name-clash
struct faust_wrapper {

    faust_wrapper(String classId):
        sampleRate(0),
        jitFactory(nullptr),
        jitDsp(nullptr),
        classId(classId)
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

    // audio buffer
    int _nChannels;
    int _nFramesMax;
    std::vector<float> inputBuffer;
    std::vector<float*> inputChannelPointers;

    bool setup() {
        // cleanup old code and factories
        if (jitDsp != nullptr) {
            delete jitDsp;
            jitDsp = nullptr;
        }
        if (jitFactory != nullptr) {
            ::faust::deleteDSPFactory(jitFactory);
            // no need to set jitFactory=nullptr, as it will be overwritten in the next step
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

    void prepare(PrepareSpecs specs)
    {
        // recompile if sample rate changed
        int newSampleRate = (int)specs.sampleRate;
        if (newSampleRate != sampleRate) {
            sampleRate = newSampleRate;
            // init samplerate
            init();
        }

        if (_nChannels != specs.numChannels || _nFramesMax != specs.blockSize) {
            std::cout << "Faust: Resizing buffers" << std::endl;
            _nChannels = specs.numChannels;
            _nFramesMax = specs.blockSize;
            resizeBuffer();
        }
    }

    void init() {
        if (jitDsp)
            jitDsp->init(sampleRate);
    }

    void reset()
    {
        if (jitDsp) {
            jitDsp->instanceClear();
        }
    }

    String getClassId() {
        return classId;
    }

    void process(ProcessDataDyn& data)
    {
        if (jitDsp) {
            // TODO: stable and sane sample format matching
            int n_faust_inputs = jitDsp->getNumInputs();
            int n_faust_outputs = jitDsp->getNumOutputs();
            int n_hise_channels = data.getNumChannels();

            if (n_faust_inputs == n_hise_channels && n_faust_outputs == n_hise_channels) {
                int nFrames = data.getNumSamples();
                float** channel_data = data.getRawDataPointers();
                // copy input data, because even with -inpl not all faust generated code can handle
                // in-place processing
                bufferChannelsData(channel_data, n_hise_channels, nFrames);
                jitDsp->compute(nFrames, getRawInputChannelPointers(), channel_data);
            } else {
                // TODO error indication
            }
        } else {
            // std::cout << "Faust: dsp was not initialized" << std::endl;
        }
    }

    float** getRawInputChannelPointers() {
        return &inputChannelPointers[0];
    }

    void resizeBuffer()
    {
        inputBuffer.resize(_nChannels * _nFramesMax);
        // setup new pointers
        inputChannelPointers.resize(_nChannels);
        inputChannelPointers.clear();
        for (int i=0; i<inputBuffer.size(); i+=_nFramesMax) {
            inputChannelPointers.push_back(&inputBuffer[i]);
        }
    }

    void bufferChannelsData(float** channels, int nChannels, int nFrames)
    {
        assert(nChannels == _nChannels);
        assert(nFrames <= _nFramesMax);

        for (int i=0; i<nChannels; i++) {
            memcpy(inputChannelPointers[i], channels[i], nFrames * sizeof(float));
        }
    }

private:
    String classId;
};

} // namespace faust
} // namespace scriptnode

#endif // __FAUST_WRAPPER_H
