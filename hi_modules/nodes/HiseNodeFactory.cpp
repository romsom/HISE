/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
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

namespace scriptnode
{

using namespace juce;
using namespace hise;

namespace analyse
{
Factory::Factory(DspNetwork* network):
	NodeFactory(network)
{
	registerNode<fft>({});
	registerNode<oscilloscope>({});
}

}


namespace dynamics
{
Factory::Factory(DspNetwork* network) :
	NodeFactory(network)
{
	registerNode<gate>();
	registerNode<comp>();
	registerNode<limiter>();
	registerNode<envelope_follower>();
}

}

namespace fx
{
Factory::Factory(DspNetwork* network) :
	NodeFactory(network)
{
	registerPolyNode<sampleandhold, sampleandhold_poly>({});
	registerPolyNode<bitcrush, bitcrush_poly>({});
	registerPolyNode<haas, haas_poly>({});
	registerPolyNode<phase_delay, phase_delay_poly>({});
	registerNode<reverb>({});
	
}



}

namespace core
{
Factory::Factory(DspNetwork* network) :
	NodeFactory(network)
{
	registerNodeRaw<SoulNode>();

	registerPolyNode<seq, seq_poly>();
	registerPolyNode<ramp, ramp_poly>();
#if HISE_INCLUDE_SNEX
	registerPolyNodeRaw<JitNode, JitPolyNode>();
	registerNode<core::simple_jit>({});
#endif
	registerNode<mono2stereo>({});
	registerNode<table>();
	registerNode<fix_delay>();
	registerNode<etxzat::etxzat_node>();
	registerNode<file_player>();
	registerNode<hise_mod>();
	registerNode<fm>();
	registerPolyNode<oscillator, oscillator_poly>();
	registerPolyNode<ramp_envelope, ramp_envelope_poly>();
	registerPolyNode<gain, gain_poly>();
	registerNode<peak>();
	registerNode<tempo_sync>();
	registerPolyNode<timer, timer_poly>();
	registerPolyNode<midi, midi_poly>({});
	registerPolyNode<smoother, smoother_poly>({});
}
}



}
