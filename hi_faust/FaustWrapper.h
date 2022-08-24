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

    void init() {
        if (jitDsp)
            jitDsp->init(sampleRate);
    }

    String getClassId() {
        return classId;
    }

private:
    String classId;
};

} // namespace faust
} // namespace scriptnode

#endif // __FAUST_WRAPPER_H
