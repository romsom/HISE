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
            std::cout << "Faust: Resizing buffers" << std::endl;
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

	static std::string prefixClassForFaust(std::string _classId) {
		return "_" + _classId;
	}

	std::string classIdForFaustClass() {
		return prefixClassForFaust(classId.toStdString());
	}

	static std::string genStaticInstanceBoilerplate(std::string dest_dir, std::string _classId) {
		std::string dest_file = _classId + ".cpp";
		std::string metaDataClass = _classId + "MetaData";
		std::string faustClassId = prefixClassForFaust(_classId);
		std::string body =
			"using Meta = faust::Meta;\n"
			"using UI = faust::UI;\n"
			"#include \"src/" + _classId + ".cpp\"\n"
			"namespace project {\n"
			"struct " + metaDataClass + " {\n"
			"		SN_NODE_ID(\"" + _classId + "\");\n"
			"};\n"
			"template <int NV>\n"
			"using " + _classId + " = scriptnode::faust::faust_static_wrapper<1, " + faustClassId + " , " + metaDataClass + ">;\n"
			"} // namespace project\n";

		auto dir = juce::File(dest_dir);
		if (!dir.isDirectory())
			return "";

		auto dest = dir.getChildFile(dest_file);
		dest.replaceWithText(body);

		DBG("Static body file generation successful: " + dest.getFullPathName());

		return dest_file;
	}

	static bool genAuxFile(std::string _classId, std::string code, int argc, const char* argv[]) {
		std::string aux_content = "none";
		std::string error_msg;

		if (!::faust::generateAuxFilesFromString(prefixClassForFaust(_classId), code, argc, argv, error_msg)) {
			// TODO replace DBG with appropriate error logging function
			DBG("hi_faust_jit: Aux file generation failed:");
			DBG("argv: ");
			while (*argv) {
				DBG(std::string("\t") + (*argv++));
			}
			DBG("result: " + error_msg);
			return false;
		}
		return true;
	}

	static std::string genStaticInstanceCode(std::string _classId, std::string code, std::string includePath, std::string dest_dir) {
		std::string dest_file = _classId + ".cpp";
		int argc = 13;
		std::string faustClassId = prefixClassForFaust(_classId);
		const char* argv[] = {"-rui", "-I", includePath.c_str(), "-lang", "cpp", "-scn", "faust::dsp", "-cn", faustClassId.c_str(), "-O", dest_dir.c_str(), "-o", dest_file.c_str(), nullptr};
		if (genAuxFile(_classId, code, argc, argv)) {
			DBG("hi_faust_jit: Static code generation successful: " + dest_file);
			return dest_file;
		}
		return "";
	}

private:
    String classId;
};

/**
 * Templated wrapper for statically generated Faust DSP class
 */
template <class FaustDsp>
struct faust_wrapper : public faust_base_wrapper {
	faust_wrapper():
		faust_base_wrapper((::faust::dsp*)(new FaustDsp))
	{ }

	~faust_wrapper() {
		delete (FaustDsp*)faustDsp;
	}
};

} // namespace faust
} // namespace scriptnode

#endif // __FAUST_BASE_WRAPPER_H
