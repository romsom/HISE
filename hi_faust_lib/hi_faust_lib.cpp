#if HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#include "faust_wrap/dsp/libfaust-c-backend-placeholder.cpp"
#else // !HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#include "faust_wrap/dsp/libfaust.cpp"
#endif // HISE_FAUST_USE_LIBFAUST_C_INTERFACE

#if HISE_FAUST_USE_LLVM_JIT
#if HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#include "faust_wrap/dsp/llvm-dsp-c-backend-placeholder.cpp"
#else // HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#include "faust_wrap/dsp/llvm-dsp.cpp"
#endif // HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#else
#if HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#include "faust_wrap/dsp/interpreter-dsp-c-backend-placeholder.cpp"
#else // !HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#include "faust_wrap/dsp/interpreter-dsp.cpp"
#endif // HISE_FAUST_USE_LIBFAUST_C_INTERFACE
#endif // HISE_FAUST_USE_LLVM_JIT
