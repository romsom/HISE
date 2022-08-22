#ifndef __FAUST_UI_H

namespace scriptnode {
namespace faust {

struct faust_ui : public ::faust::UI {
    enum ControlType {
        NONE = 0,
        BUTTON,
        CHECK_BUTTON,
        VERTICAL_SLIDER,
        HORIZONTAL_SLIDER,
        NUM_ENTRY,
        HORIZONTAL_BARGRAPH,
        VERTICAL_BARGRAPH,
        // SOUND_FILE, // Handle Soundfile separately
        MIDI,
        OTHER=0xffff,
    };

    struct Parameter {
        ControlType type;
        String label;
        float* zone;
        float init;
        float min;
        float max;
        float step;

        Parameter(ControlType type,
                  String label,
                  float* zone,
                  float init,
                  float min,
                  float max,
                  float step) :
            type(type),
            label(label),
            zone(zone),
            init(init),
            min(min),
            max(max),
            step(step) {}

    };

    faust_ui() { }

    std::vector<std::shared_ptr<Parameter>> parameters;

    std::optional<std::shared_ptr<Parameter>> getParameterByLabel(String label)
    {
        for (auto p : parameters)
        {
            if (p->label == label)
                return p;
        }
        return {};
    }

    std::vector<String> getParameterLabels()
    {
        std::vector<String> res;
        res.reserve(parameters.size());

        for (auto p : parameters)
        {
            res.push_back(p->label);
        }

        return res;
    }

    float* getZone(String label) {
        for (auto p : parameters)
        {
            if (p->label == label)
                return p->zone;
        }
        return nullptr;
    }


    // -- metadata declarations

    virtual void declare(float* zone, const char* key, const char* val) override
    {
        // TODO
    }

    // Faust UI implementation

    virtual void openTabBox(const char* label) override
    {

    }
    virtual void openHorizontalBox(const char* label) override
    {

    }
    virtual void openVerticalBox(const char* label) override
    {

    }
    virtual void closeBox() override { }

    // -- active widgets

    virtual void addButton(const char* label, float* zone) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::BUTTON,
                                                         String(label),
                                                         zone,
                                                         0.f,
                                                         0.f,
                                                         1.f,
                                                         1.f));
    }
    virtual void addCheckButton(const char* label, float* zone) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::CHECK_BUTTON,
                                                         String(label),
                                                         zone,
                                                         0.f,
                                                         0.f,
                                                         1.f,
                                                         1.f));

    }
    virtual void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::VERTICAL_SLIDER,
                                                         String(label),
                                                         zone,
                                                         init,
                                                         min,
                                                         max,
                                                         step));
    }
    virtual void addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::HORIZONTAL_SLIDER,
                                                         String(label),
                                                         zone,
                                                         init,
                                                         min,
                                                         max,
                                                         step));
    }
    virtual void addNumEntry(const char* label, float* zone, float init, float min, float max, float step) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::NUM_ENTRY,
                                                         String(label),
                                                         zone,
                                                         init,
                                                         min,
                                                         max,
                                                         step));
    }

    // -- passive widgets

    virtual void addHorizontalBargraph(const char* label, float* zone, float min, float max) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::HORIZONTAL_BARGRAPH,
                                                         String(label),
                                                         zone,
                                                         0.f,
                                                         min,
                                                         max,
                                                         1.f));
    }
    virtual void addVerticalBargraph(const char* label, float* zone, float min, float max) override
    {
        parameters.push_back(std::make_shared<Parameter>(ControlType::VERTICAL_BARGRAPH,
                                                         String(label),
                                                         zone,
                                                         0.f,
                                                         min,
                                                         max,
                                                         1.f));
    }

    // -- soundfiles -- TODO

    virtual void addSoundfile(const char* label, const char* filename, ::faust::Soundfile** sf_zone) { }

};

} // namespace faust
} // namespace scriptnode

#endif // __FAUST_UI_H
