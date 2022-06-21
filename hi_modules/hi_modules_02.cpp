
#include "JuceHeader.h"


#include "nodes/ComplexDataNodes.h"
#include "nodes/ConvolutionNode.h"


#include "nodes/ComplexDataNodes.cpp"
#include "nodes/ConvolutionNode.cpp"


#if HISE_INCLUDE_FAUST
#include "faust/FaustNode.cpp"
#endif // HISE_INCLUDE_FAUST


#include "nodes/HiseNodeFactory.cpp"
