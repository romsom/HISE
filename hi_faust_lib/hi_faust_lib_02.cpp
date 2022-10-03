// This is the land of the faust C interface, which is needed for windows

#if HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#include "faust_wrap/dsp/libfaust-c-backend.cpp"
#if HISE_FAUST_USE_LLVM_JIT
#include "faust_wrap/dsp/llvm-dsp-c-backend.cpp"
#else // !HISE_FAUST_USE_LLVM_JIT
#include "faust_wrap/dsp/interpreter-dsp-c-backend.cpp"
#endif // HISE_FAUST_USE_LLVM_JIT
#endif // HISE_FAUST_USE_LIBFAUST_C_INTERFACE
