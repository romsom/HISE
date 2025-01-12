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
 *   which also must be licenced for commercial applications:
 *
 *   http://www.juce.com
 *
 *   ===========================================================================
 */

#pragma once

namespace scriptnode
{
using namespace juce;
using namespace hise;



    
class ModulationSourceNode: public NodeBase,
							public SnexDebugHandler
{
public:

	static constexpr int ModulationBarHeight = 60;
	static constexpr int RingBufferSize = 65536;

	struct ModulationTarget: public ConnectionBase
	{
		static void nothing(double) {}

		ModulationTarget(ModulationSourceNode* parent_, ValueTree data_);

		~ModulationTarget();

		bool findTarget();

		void setExpression(const String& exprCode);

		void applyValue(double value);

		bool isModulationConnection() const override { return true; }

		valuetree::PropertyListener expressionUpdater;
		valuetree::PropertyListener rangeUpdater;
		valuetree::RemoveListener removeWatcher;
		

		WeakReference<ModulationSourceNode> parent;
		
		
		DspHelpers::ParameterCallback callback;

		CachedValue<bool> active;
		
		SnexExpressionPtr expr;
		SpinLock expressionLock;
	};

	ValueTree getModulationTargetTree();;

	ModulationSourceNode(DspNetwork* n, ValueTree d);;

	var addModulationTarget(NodeBase::Parameter* n);

	String createCppClass(bool isOuterClass) override;
	
	void prepare(PrepareSpecs ps) override;
	void sendValueToTargets(double value, int numSamplesForAnalysis);;

	void logMessage(const String& s) override;


	int fillAnalysisBuffer(AudioSampleBuffer& b);
	void setScaleModulationValue(bool shouldScaleValue);

	bool shouldScaleModulationValue() const noexcept { return scaleModulationValue; }

	virtual bool isUsingModulation() const { return false; }

private:

	void checkTargets();

	double sampleRateFactor = 1.0;
	bool prepareWasCalled = false;

	double lastModValue = 0.0;

	bool scaleModulationValue = true;

	int ringBufferSize = 0;

	struct SimpleRingBuffer
	{
		SimpleRingBuffer();

		void clear();
		int read(AudioSampleBuffer& b);
		void write(double value, int numSamples);

		bool isBeingWritten = false;

		int numAvailable = 0;
		int writeIndex = 0;
		float buffer[RingBufferSize];
	};

	ScopedPointer<SimpleRingBuffer> ringBuffer;

	valuetree::ChildListener targetListener;

	bool ok = false;

	ReferenceCountedArray<ModulationTarget> targets;
	JUCE_DECLARE_WEAK_REFERENCEABLE(ModulationSourceNode);
};


struct ModulationSourceBaseComponent : public Component,
	public PooledUIUpdater::SimpleTimer
{
	ModulationSourceBaseComponent(PooledUIUpdater* updater);;
	ModulationSourceNode* getSourceNodeFromParent() const;

	void timerCallback() override {};
	void paint(Graphics& g) override;
	Image createDragImage();

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent&) override;

protected:

	mutable WeakReference<ModulationSourceNode> sourceNode;
};


struct ModulationSourcePlotter : ModulationSourceBaseComponent
{
	ModulationSourcePlotter(PooledUIUpdater* updater);

	void timerCallback() override;
	void rebuildPath();
	void paint(Graphics& g) override;

	int getSamplesPerPixel() const;

	float pixelCounter = 0.0f;
	bool skip = false;

	RectangleList<float> rectangles;
	AudioSampleBuffer buffer;
};

}
