#define FAUSTFLOAT float

#include <faust_wrap/dsp/llvm-dsp.h>
#include <faust_wrap/gui/UI.h>

namespace scriptnode {
namespace faust {

faust_jit_node::faust_jit_node(DspNetwork* n, ValueTree v) :
	faust_base_node(n, v)
{
}

// void faust_jit_node::setupParameters()
// {
//     // setup parameters from faust code
//     for (auto p : faust->ui.parameters)
//     {
//         DBG("adding parameter " << p->label << ", step: " << p->step);
//         switch (p->type) {
//         case faust_jit_ui::ControlType::VERTICAL_SLIDER:
//         case faust_jit_ui::ControlType::HORIZONTAL_SLIDER:
//         case faust_jit_ui::ControlType::NUM_ENTRY:
//         {
//             parameter::data pd(p->label, {(double)(p->min), (double)(p->max), (double)(p->step)});
//             pd.setDefaultValue((double)(p->init));
//             addNewParameter(pd);
//         }
//         break;
//         case faust_jit_ui::ControlType::CHECK_BUTTON:
//         {
//             parameter::data pd(p->label, {0.0, 1.0});
//             pd.setDefaultValue((double)(p->init));
//             pd.setParameterValueNames({"off", "on"});
//             addNewParameter(pd);
//         }
//         break;
//         }
//     }
//     DBG("Num parameters in NodeBase: " << getNumParameters());
// }

// static void updateFaustZone(void* obj, double value)
// {
//     *static_cast<float*>(obj) = (float)value;
// }

// ParameterTree listener callback: This function is called when the ParameterTree changes
// void faust_jit_node::parameterUpdated(ValueTree child, bool wasAdded)
// {
//     if (wasAdded)
//     {
//         String parameterLabel = child.getProperty(PropertyIds::ID);
//         auto faustParameter = faust->ui.getParameterByLabel(parameterLabel).value_or(nullptr);

//         // proceed only if this parameter was defined in faust (check by label)
//         if (!faustParameter)
//             return;

//         float* zonePointer = faustParameter->zone;

//         // setup dynamic parameter
//         auto dp = new scriptnode::parameter::dynamic();
//         // install callback and zone pointer into dynamic parameter
//         dp->referTo(zonePointer, updateFaustZone);

//         // create dynamic_base from dynamic parameter to attach to the new Parameter
//         auto dyn_base = new scriptnode::parameter::dynamic_base(*dp);

//         // create and setup parameter
//         auto p = new scriptnode::Parameter(this, child);
//         p->setDynamicParameter(dyn_base);

//         NodeBase::addParameter(p);
//         {
//             auto index = child.getParent().indexOf(child);
//             DBG("parameter added, index=" << index);
//         }
//     }
//     else
//     {
//         String parameterLabel = child.getProperty(PropertyIds::ID);
//         DBG("removing parameter: ");
//         DBG(parameterLabel);
//         NodeBase::removeParameter(parameterLabel);
//     }
// }

// void faust_jit_node::initialise(NodeBase* n)
// {
//     classId.initialise(n);
//     classId.setAdditionalCallback(BIND_MEMBER_FUNCTION_2(faust_jit_node::updateClassId), true);
// }

// void faust_jit_node::prepare(PrepareSpecs specs)
// {
//     lastSpecs = specs;
//     faust->prepare(specs);
// }

NodeBase* faust_jit_node::createNode(DspNetwork* n, ValueTree v)
{
    return new faust_jit_node(n, v);
}



// void faust_jit_node::addNewParameter(parameter::data p)
// {
//     if (auto existing = getParameterFromName(p.info.getId()))
//         return;

//     auto newTree = p.createValueTree();
//     getParameterTree().addChild(newTree, -1, getUndoManager());
// }

// Remove all HISE Parameters. Parameters will still be present in faust->ui
// and will be cleared in faust->setup() automatically
// void faust_jit_node::resetParameters()
// {
//     DBG("Resetting parameters");
//     getParameterTree().removeAllChildren(getUndoManager());
// }



} // namespace faust
} // namespace scriptnode
