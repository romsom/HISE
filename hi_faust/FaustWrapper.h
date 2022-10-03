#include <atomic>
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#ifndef __FAUST_BASE_WRAPPER_H
#define __FAUST_BASE_WRAPPER_H

namespace scriptnode {
namespace faust {

// wrapper struct for faust types to avoid name-clash
struct faust_base_wrapper {

	faust_base_wrapper(::faust::dsp* faustDsp):
	    faustDsp(faustDsp),  // Unless faustDsp is set to a non-nullptr value in a subclass, several methods will cause a segfault
        sampleRate(0),
        _nChannels(0),
        _nFramesMax(0)
    {
	    // faustDsp will be instantiated in templated class here
    }

    ~faust_base_wrapper()
    {
    }

	// std::string code;
    int sampleRate;
    ::faust::dsp *faustDsp;
    faust_ui ui;

    // audio buffer
    int _nChannels;
    int _nFramesMax;
    std::vector<float> inputBuffer;
    std::vector<float*> inputChannelPointers;

    bool setup() {
        faustDsp->buildUserInterface(&ui);

        DBG("Faust parameters:");
        for (auto p : ui.getParameterLabels()) {
            DBG(p);
        }

        init();
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
            DBG("Faust: Resizing buffers");
            _nChannels = specs.numChannels;
            _nFramesMax = specs.blockSize;
            resizeBuffer();
        }
    }

    void init() {
	    faustDsp->init(sampleRate);
    }

    void reset()
    {
	    faustDsp->instanceClear();
    }

    String getClassId() {
        return classId;
    }

    void process(ProcessDataDyn& data)
    {
	    // we have either a static ::faust::dsp object here, or we hold the jit lock
	    // TODO: stable and sane sample format matching
	    int n_faust_inputs = faustDsp->getNumInputs();
	    int n_faust_outputs = faustDsp->getNumOutputs();
	    int n_hise_channels = data.getNumChannels();

	    if (n_faust_inputs == n_hise_channels && n_faust_outputs == n_hise_channels) {
		    int nFrames = data.getNumSamples();
		    float** channel_data = data.getRawDataPointers();
		    // copy input data, because even with -inpl not all faust generated code can handle
		    // in-place processing
		    bufferChannelsData(channel_data, n_hise_channels, nFrames);
		    faustDsp->compute(nFrames, getRawInputChannelPointers(), channel_data);
	    } else {
		    // TODO error indication
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

#endif // __FAUST_BASE_WRAPPER_H
