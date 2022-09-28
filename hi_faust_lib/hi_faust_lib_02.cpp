// This is the land of the faust C interface, which is needed for windows

#if !HISE_FAUST_USE_LLVM_JIT
#include "faust_wrap/dsp/interpreter-dsp-c-backend.cpp"
#include "faust_wrap/dsp/libfaust-c-backend.cpp"
#endif
