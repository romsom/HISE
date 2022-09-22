#include <atomic>
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#ifndef __FAUST_JIT_WRAPPER_H
#define __FAUST_JIT_WRAPPER_H

namespace scriptnode {
namespace faust {

// wrapper struct for faust types to avoid name-clash
struct faust_jit_wrapper : public faust_base_wrapper {

    faust_jit_wrapper(String classId, String projectDir):
	    faust_base_wrapper(nullptr),
        jitFactory(nullptr),
        classId(classId),
        projectDir(projectDir)
    { }

    ~faust_jit_wrapper()
    {
        if(faustDsp)
            delete faustDsp;
        if (jitFactory)
            ::faust::deleteDSPFactory(jitFactory);
    }

    std::string code;
    std::string errorMessage;
    int jitOptimize = 0; // -1 is maximum optimization
    ::faust::llvm_dsp_factory* jitFactory;
    String projectDir;

    // Mutex for synchronization of compilation and processing
    juce::CriticalSection jitLock;


    bool setup() {
        juce::ScopedLock sl(jitLock);
        // because the audio thread is real-time, we can wait for the duration of one
        // frame and be sure we don't modify any data the audio thread still uses
        // TODO: calculate actual duration
        std::this_thread::sleep_for(20ms);

        // cleanup old code and factories
        // make sure faustDsp is nullptr in case we fail to recompile
        // so we don't use an old deallocated faustDsp in process (checks
        // for faustDsp == nullptr)
        if (faustDsp != nullptr) {
            delete faustDsp;
            faustDsp = nullptr;
        }
        if (jitFactory != nullptr) {
            ::faust::deleteDSPFactory(jitFactory);
            // no need to set jitFactory=nullptr, as it will be overwritten in the next step
        }

        ui.reset();

        int llvm_argc = 3;
        const char* llvm_argv[] = {"-rui", "-I", projectDir.toRawUTF8(), nullptr};

        jitFactory = ::faust::createDSPFactoryFromString("faust", code, llvm_argc, llvm_argv, "",
                                                         errorMessage, jitOptimize);
        if (jitFactory == nullptr) {
            // TODO error indication
            std::cout << "Faust jit compilation failed:" << std::endl
                      << errorMessage << std::endl;
            return false;
        }
        std::cout << "Faust jit compilation successful" << std::endl;
        faustDsp = jitFactory->createDSPInstance();
        if (faustDsp == nullptr) {
            std::cout << "Faust DSP instantiation" << std::endl;
            return false;
        }

        std::cout << "Faust DSP instantiation successful" << std::endl;

        faust_base_wrapper::setup();
        
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
        if (faustDsp)
            faustDsp->init(sampleRate);
    }

    void reset()
    {
        if (faustDsp) {
            faustDsp->instanceClear();
        }
    }

    String getClassId() {
        return classId;
    }

	void setCode(String newClassId, std::string newCode) {
		classId = newClassId;
		code = newCode;
	}

    void process(ProcessDataDyn& data)
    {
	    // run jitted code only while holding the corresponding lock:
        juce::ScopedTryLock stl(jitLock);
        if (stl.isLocked() && faustDsp) {
	        faust_base_wrapper::process(data);
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

#endif // __FAUST_JIT_WRAPPER_H
