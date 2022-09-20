#define FAUSTFLOAT float

#include <faust_wrap/dsp/llvm-dsp.h>
#include <faust_wrap/gui/UI.h>

#include "FaustUI.h"
#include "FaustWrapper.h"
#include "FaustMenuBar.h"

namespace scriptnode {
namespace faust {

// faust_base_node::faust_base_node(DspNetwork* n, ValueTree v) :
//      NodeBase(n, v, 0) { }
faust_base_node::faust_base_node(DspNetwork* n, ValueTree v, faust_base_wrapper* faustPtr) :
    WrapperNode(n, v),
    faust(faustPtr)
{
    extraComponentFunction = [](void* o, PooledUIUpdater* u)
    {
        return new FaustMenuBar(static_cast<faust_base_node*>(o));
    };

    parameterListener.setCallback(getParameterTree(),
                                  valuetree::AsyncMode::Synchronously,
                                  BIND_MEMBER_FUNCTION_2(faust_base_node::parameterUpdated));
}

void faust_base_node::setupParameters()
{
    // setup parameters from faust code
    for (auto p : faust->ui.parameters)
    {
        DBG("adding parameter " << p->label << ", step: " << p->step);
        auto pd = p->toParameterData();
        addNewParameter(pd);
    }
    DBG("Num parameters in NodeBase: " << getNumParameters());
}

static void updateFaustZone(void* obj, double value)
{
    *static_cast<float*>(obj) = (float)value;
}

// ParameterTree listener callback: This function is called when the ParameterTree changes
void faust_base_node::parameterUpdated(ValueTree child, bool wasAdded)
{
    if (wasAdded)
    {
        String parameterLabel = child.getProperty(PropertyIds::ID);
        auto faustParameter = faust->ui.getParameterByLabel(parameterLabel).value_or(nullptr);

        // proceed only if this parameter was defined in faust (check by label)
        if (!faustParameter)
            return;

        float* zonePointer = faustParameter->zone;

        // setup dynamic parameter
        auto dp = new scriptnode::parameter::dynamic();
        // install callback and zone pointer into dynamic parameter
        dp->referTo(zonePointer, updateFaustZone);

        // create dynamic_base from dynamic parameter to attach to the new Parameter
        auto dyn_base = new scriptnode::parameter::dynamic_base(*dp);

        // create and setup parameter
        auto p = new scriptnode::Parameter(this, child);
        p->setDynamicParameter(dyn_base);

        NodeBase::addParameter(p);
        {
            auto index = child.getParent().indexOf(child);
            DBG("parameter added, index=" << index);
        }
    }
    else
    {
        String parameterLabel = child.getProperty(PropertyIds::ID);
        DBG("removing parameter: ");
        DBG(parameterLabel);
        NodeBase::removeParameter(parameterLabel);
    }
}

void faust_base_node::initialise(NodeBase* n)
{ }


void faust_base_node::prepare(PrepareSpecs specs)
{
    lastSpecs = specs;
    faust->prepare(specs);
}

void faust_base_node::reset()
{
    faust->reset();
}

void faust_base_node::process(ProcessDataDyn& data)
{
    if (isBypassed()) return;
    faust->process(data);
}

void faust_base_node::processFrame(FrameType& data)
{ }

String faust_base_node::getClassId()
{
	return "invalid/faust_base_node";
}
void faust_base_node::setClass(const String& newClassId) { }

StringArray faust_base_node::getAvailableClassIds()
{
	return StringArray();
}

void faust_base_node::addNewParameter(parameter::data p)
{
    if (auto existing = getParameterFromName(p.info.getId()))
        return;

    auto newTree = p.createValueTree();
    getParameterTree().addChild(newTree, -1, getUndoManager());
}

// Remove all HISE Parameters. Parameters will still be present in faust->ui
// and will be cleared in faust->setup() automatically
void faust_base_node::resetParameters()
{
    DBG("Resetting parameters");
    getParameterTree().removeAllChildren(getUndoManager());
}


} // namespace faust
} // namespace scriptnode
