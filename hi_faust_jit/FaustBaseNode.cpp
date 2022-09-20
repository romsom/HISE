#define FAUSTFLOAT float

#include <faust_wrap/dsp/llvm-dsp.h>
#include <faust_wrap/gui/UI.h>
#include "FaustWrapper.h"
#include "FaustMenuBar.h"

namespace scriptnode {
namespace faust {

// faust_base_node::faust_base_node(DspNetwork* n, ValueTree v) :
//      NodeBase(n, v, 0) { }
faust_base_node::faust_base_node(DspNetwork* n, ValueTree v) :
	WrapperNode(n, v),
    classId(PropertyIds::ClassId, "faust"),
    faust(new faust_jit_wrapper("faust", getFaustRootFile().getFullPathName()))
{
    extraComponentFunction = [](void* o, PooledUIUpdater* u)
    {
        return new FaustMenuBar(static_cast<faust_base_node*>(o));
    };

    parameterListener.setCallback(getParameterTree(),
                                  valuetree::AsyncMode::Synchronously,
                                  BIND_MEMBER_FUNCTION_2(faust_base_node::parameterUpdated));

    File f = getFaustRootFile();
    // Create directory if it's not already there
    if (!f.isDirectory()) {
        auto res = f.createDirectory();
    }
    DBG(f.getFullPathName());
    // we can't init yet, because we don't know the sample rate
    setClass(classId.getValue());
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

File faust_base_node::getFaustRootFile()
{
    auto mc = this->getScriptProcessor()->getMainController_();
    auto dspRoot = mc->getCurrentFileHandler().getSubDirectory(FileHandlerBase::DspNetworks);
    return dspRoot.getChildFile("CodeLibrary/faust");
}

/*
 * Lookup a Faust source code file for this node.
 * The `basename` is the name without any extension.
 */
File faust_base_node::getFaustFile(String basename)
{
    auto nodeRoot = getFaustRootFile();
    return nodeRoot.getChildFile(basename + ".dsp");
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

String faust_base_node::getClassId()
{
    return classId.getValue();
}

void faust_base_node::loadSource()
{
    auto newClassId = getClassId();
    if (faust->getClassId() == newClassId) return;
    File sourceFile = getFaustFile(newClassId);

    // Create new file if necessary
    if (!sourceFile.existsAsFile()) {
        auto res = sourceFile.create();
        if (res.failed()) {
            std::cerr << "Failed creating file \"" + sourceFile.getFullPathName() + "\"" << std::endl;
        }
    }

    DBG("Faust DSP file to load:" << sourceFile.getFullPathName());

    // Load file and recompile
    String code = sourceFile.loadFileAsString();
    faust->setCode(code.toStdString());
    // setup dsp
    bool success = faust->setup();
    std::cout << "Faust initialization: " << (success ? "success" : "failed") << std::endl;
    // TODO: error handling
    if (!success)
    {
        auto p = dynamic_cast<Processor*>(getScriptProcessor());

        debugError(p, "FaustError");
    }
    setupParameters();
}

void faust_base_node::setClass(const String& newClassId)
{
    classId.storeValue(newClassId, getUndoManager());
    updateClassId({}, newClassId);
    resetParameters();
    loadSource();
}

void faust_base_node::updateClassId(Identifier, var newValue)
{
    auto newId = newValue.toString();

    DBG(newId);
    if (newId.isNotEmpty())
    {
        auto nb = getRootNetwork()->codeManager.getOrCreate(getTypeId(), Identifier(newValue.toString()));
        // TODO: workbench
    }
}

StringArray faust_base_node::getAvailableClassIds()
{
    return getRootNetwork()->codeManager.getClassList(getTypeId());
}


} // namespace faust
} // namespace scriptnode
