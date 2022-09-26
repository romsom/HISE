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

	static std::string prefixClassForFaust(std::string _classId) {
		return "_" + _classId;
	}

	std::string classIdForFaustClass() {
		return prefixClassForFaust(classId.toStdString());
	}

	static std::string genStaticInstanceBoilerplate(std::string dest_dir, std::string _classId) {
		std::string dest_file = _classId + ".h";
		std::string metaDataClass = _classId + "MetaData";
		std::string faustClassId = prefixClassForFaust(_classId);
		std::string body =
			"#pragma once\n"
			"#include \"hi_faust_types/hi_faust_types.h\"\n"
			"using Meta = faust::Meta;\n"
			"using UI = faust::UI;\n"
			"#define FAUST_UIMACROS\n"
			" // define dummy macros\n"
			"#define FAUST_ADDCHECKBOX(...)\n"
			"#define FAUST_ADDNUMENTRY(...)\n"
			"#define FAUST_ADDBUTTON(...)\n"
			"#define FAUST_ADDHORIZONTALSLIDER(...)\n"
			"#define FAUST_ADDVERTICALSLIDER(...)\n"
			"#define FAUST_ADDVERTICALBARGRAPH(...)\n"
			"#define FAUST_ADDHORIZONTALBARGRAPH(...)\n"
			"#define FAUST_ADDSOUNDFILE(...)\n"
			"#include \"src/" + _classId + ".cpp\"\n"
			"#if FAUST_INPUTS - FAUST_OUTPUTS\n"
			"#error Number of inputs and outputs in faust code must match!\n"
			"#endif\n"
			"namespace project {\n"
			"struct " + metaDataClass + " {\n"
			"		SN_NODE_ID(\"" + _classId + "\");\n"
			"};\n"
			"template <int NV>\n"
			"using " + _classId + " = scriptnode::faust::faust_static_wrapper<1, " + faustClassId + " , " + metaDataClass + ", FAUST_INPUTS>;\n"
			"} // namespace project\n"
			" // undef dummy macros\n"
			"#undef FAUST_UIMACROS\n"
			"#undef FAUST_ADDCHECKBOX\n"
			"#undef FAUST_ADDNUMENTRY\n"
			"#undef FAUST_ADDBUTTON\n"
			"#undef FAUST_ADDHORIZONTALSLIDER\n"
			"#undef FAUST_ADDVERTICALSLIDER\n"
			"#undef FAUST_ADDVERTICALBARGRAPH\n"
			"#undef FAUST_ADDHORIZONTALBARGRAPH\n"
			"#undef FAUST_ADDSOUNDFILE\n"
			" // undef faust ui macros\n"
			"#undef FAUST_FILE_NAME\n"
			"#undef FAUST_CLASS_NAME\n"
			"#undef FAUST_COMPILATION_OPIONS\n"
			"#undef FAUST_INPUTS\n"
			"#undef FAUST_OUTPUTS\n"
			"#undef FAUST_ACTIVES\n"
			"#undef FAUST_PASSIVES\n"
			"#undef FAUST_LIST_ACTIVES\n"
			"#undef FAUST_LIST_PASSIVES\n";

		auto dir = juce::File(dest_dir);
		if (!dir.isDirectory())
			return "";

		auto dest = dir.getChildFile(dest_file);
		dest.replaceWithText(body);

		DBG("Static body file generation successful: " + dest.getFullPathName());

		return dest_file;
	}

	static bool genAuxFile(std::string srcPath, int argc, const char* argv[]) {
		std::string aux_content = "none";
		std::string error_msg;

		if (!::faust::generateAuxFilesFromFile(srcPath, argc, argv, error_msg)) {
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

	static std::string genStaticInstanceCode(std::string _classId, std::string srcPath, std::string includePath, std::string dest_dir) {
		std::string dest_file = _classId + ".cpp";
		int argc = 15;
		std::string faustClassId = prefixClassForFaust(_classId);
		const char* argv[] = {"-uim", "-nvi", "-rui", "-I", includePath.c_str(), "-lang", "cpp", "-scn", "faust::dsp", "-cn", faustClassId.c_str(), "-O", dest_dir.c_str(), "-o", dest_file.c_str(), nullptr};
		if (genAuxFile(srcPath, argc, argv)) {
			DBG("hi_faust_jit: Static code generation successful: " + dest_file);
			return dest_file;
		}
		return "";
	}

private:
	String classId;
};

} // namespace faust
} // namespace scriptnode

#endif // __FAUST_JIT_WRAPPER_H
