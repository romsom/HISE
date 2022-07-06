/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2022 Roman Sommer
*   Copyright 2022 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

/******************************************************************************

BEGIN_JUCE_MODULE_DECLARATION

  ID:               hi_faust
  vendor:           Hart Instruments
  version:          0.0.1
  name:             HISE Faust Integration
  description:      All processors for HISE
  website:          http://hise.audio
  license:          GPL

  dependencies:      hi_dsp_library

  linuxLibs: faust_wrap

END_JUCE_MODULE_DECLARATION

******************************************************************************/
#ifndef HI_FAUST_INCLUDED
#define HI_FAUST_INCLUDED

/** Config: HISE_INCLUDE_FAUST

Enables the Faust Compiler
*/
#ifndef HISE_INCLUDE_FAUST
#define HISE_INCLUDE_FAUST 1
#endif // HISE_INCLUDE_FAUST


#if HISE_INCLUDE_FAUST
#include <optional>
#include "../hi_dsp_library/hi_dsp_library.h" // NodeBase
#include "../hi_core/hi_core.h" // FileHandlerBase
#include "../hi_scripting/hi_scripting.h" // DspNetwork
#include "FaustNode.h"
#endif // HISE_INCLUDE_FAUST

#endif // HI_FAUST_INCLUDED
