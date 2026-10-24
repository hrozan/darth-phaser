#include "PluginEditor.h"

#include "Parameters.h"
#include "PresetManager.h"
#include "gui/IconButton.h"
#include "gui/Knob.h"
#include "gui/LevelMeter.h"
#include "gui/LfoScope.h"
#include "gui/PanelSection.h"
#include "gui/SegmentedControl.h"
#include "gui/StereoOffsetDisplay.h"
#include "gui/Theme.h"

namespace
{
// The helmet mark, shared with design/darth-phaser-layout.html.
constexpr auto helmetSvg = R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64">
  <defs>
    <linearGradient id="hm" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#4a4a54"/><stop offset="38%" stop-color="#1b1b21"/>
      <stop offset="1" stop-color="#050507"/>
    </linearGradient>
    <linearGradient id="lens" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#ff5563"/><stop offset="1" stop-color="#a10d19"/>
    </linearGradient>
  </defs>
  <path d="M32 4c-10 0-17 7-18 17l-1 11c-1 7-3 13-6 19l7 4c5 2 11 3 18 3s13-1 18-3l7-4c-3-6-5-12-6-19l-1-11c-1-10-8-17-18-17z"
        fill="url(#hm)" stroke="#ff2233" stroke-width="1.1" stroke-opacity=".45"/>
  <path d="M13 24c5-5 11-7 19-7s14 2 19 7" fill="none" stroke="#000" stroke-width="2.4"/>
  <path d="M13 23c5-5 11-7 19-7s14 2 19 7" fill="none" stroke="#5b5b66" stroke-width=".9" stroke-opacity=".8"/>
  <path d="M15 28l12-3 1 8-11 2z" fill="url(#lens)"/>
  <path d="M49 28l-12-3-1 8 11 2z" fill="url(#lens)"/>
  <path d="M15 28l12-3 1 8-11 2z" fill="none" stroke="#000" stroke-width="1.2"/>
  <path d="M49 28l-12-3-1 8 11 2z" fill="none" stroke="#000" stroke-width="1.2"/>
  <path d="M32 20l-4 19h8z" fill="#0a0a0d" stroke="#3a3a44" stroke-width=".7"/>
  <path d="M24 40h16l-2 11H26z" fill="#07070a" stroke="#3a3a44" stroke-width=".8"/>
  <path d="M27 43h10M27.4 46h9.2M27.8 49h8.4" stroke="#ff7a18" stroke-width="1" stroke-opacity=".8"/>
  <path d="M14 38h7l-1 6h-7zM50 38h-7l1 6h7z" fill="#0a0a0d" stroke="#33333c" stroke-width=".7"/>
</svg>)SVG";

juce::RangedAudioParameter& paramRef (juce::AudioProcessorValueTreeState& state, const char* id)
{
    auto* p = state.getParameter (id);
    jassert (p != nullptr);
    return *p;
}

/** Caption above a segmented control. */
class Caption final : public juce::Component
{
public:
    explicit Caption (juce::String t) : text (std::move (t)) { setInterceptsMouseClicks (false, false); }

    void paint (juce::Graphics& g) override
    {
        g.setColour (Theme::textDim);
        g.setFont (Theme::label (9.0f, true, 0.2f));
        g.drawText (text, getLocalBounds(), juce::Justification::centredLeft, false);
    }

private:
    juce::String text;
};

/** The power LED in the header; lit means not bypassed. */
class PowerLed final : public juce::Component
{
public:
    explicit PowerLed (juce::RangedAudioParameter& p)
        : attachment (p, [this] (float v) { on = v < 0.5f; repaint(); }, nullptr)
    {
        attachment.sendInitialUpdate();
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        attachment.setValueAsCompleteGesture (on ? 1.0f : 0.0f);
    }

    void paint (juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        auto ledArea = area.removeFromTop (area.getHeight() - 12);
        const auto led = juce::Rectangle<float> (11.0f, 11.0f)
                             .withCentre (ledArea.toFloat().getCentre());

        if (on)
        {
            g.setColour (Theme::saber.withAlpha (0.35f));
            g.fillEllipse (led.expanded (9.0f));
            g.setColour (Theme::saber.withAlpha (0.55f));
            g.fillEllipse (led.expanded (4.0f));
        }

        g.setGradientFill ({ on ? juce::Colour (0xffff8a92) : juce::Colour (0xff4a1a1e),
                             led.getX() + led.getWidth() * 0.36f, led.getY() + led.getHeight() * 0.32f,
                             on ? juce::Colour (0xff7d0a13) : juce::Colour (0xff1a0608),
                             led.getCentreX(), led.getBottom(), true });
        g.fillEllipse (led);

        g.setColour (Theme::textDim);
        g.setFont (Theme::label (8.0f, true, 0.2f));
        g.drawText ("POWER", area, juce::Justification::centredTop, false);
    }

private:
    juce::ParameterAttachment attachment;
    bool on = true;
};

/** Preset name display with prev/next arrows, backed by the PresetManager. */
class PresetBar final : public juce::Component
{
public:
    explicit PresetBar (PresetManager& m) : manager (m)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        constexpr int arrow = 28;

        if (e.x < arrow)
            manager.step (-1);
        else if (e.x > getWidth() - arrow)
            manager.step (1);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        constexpr float arrow = 28.0f;

        g.setColour (juce::Colour (0xff0b0b0e));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (juce::Colours::black);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

        const auto display = bounds.reduced (arrow, 1.0f);
        g.setColour (Theme::displayBg);
        g.fillRect (display);

        g.setColour (Theme::amberBright.withAlpha (0.25f));
        g.drawVerticalLine (static_cast<int> (display.getX()), display.getY(), display.getBottom());
        g.drawVerticalLine (static_cast<int> (display.getRight()), display.getY(), display.getBottom());

        g.setColour (Theme::amberBright);
        g.setFont (Theme::label (11.0f, true, 0.2f));
        g.drawText (manager.getCurrentName().toUpperCase(), display.toNearestInt(),
                    juce::Justification::centred, false);

        g.setColour (Theme::textDim);
        g.setFont (juce::FontOptions (13.0f));
        g.drawText (juce::String::fromUTF8 ("\xe2\x97\x82"),
                    bounds.removeFromLeft (arrow).toNearestInt(), juce::Justification::centred, false);
        g.drawText (juce::String::fromUTF8 ("\xe2\x96\xb8"),
                    bounds.removeFromRight (arrow).toNearestInt(), juce::Justification::centred, false);
    }

private:
    PresetManager& manager;
};

/** FREE plus the tempo divisions in one row, driving both sync and division. */
class SyncSelector final : public juce::Component
{
public:
    SyncSelector (juce::RangedAudioParameter& syncParam, juce::RangedAudioParameter& divisionParam)
        : segment (Params::syncChoices),
          syncAttachment (syncParam,
                          [this] (float v) { syncOn = v > 0.5f; refresh(); }, nullptr),
          divisionAttachment (divisionParam,
                              [this] (float v) { division = juce::roundToInt (v); refresh(); }, nullptr)
    {
        addAndMakeVisible (segment);

        segment.onSelect = [this] (int index)
        {
            if (index == 0)
            {
                syncAttachment.setValueAsCompleteGesture (0.0f);
            }
            else
            {
                divisionAttachment.setValueAsCompleteGesture (static_cast<float> (index - 1));
                syncAttachment.setValueAsCompleteGesture (1.0f);
            }
        };

        syncAttachment.sendInitialUpdate();
        divisionAttachment.sendInitialUpdate();
    }

    void resized() override { segment.setBounds (getLocalBounds()); }

private:
    void refresh() { segment.setSelectedIndex (syncOn ? division + 1 : 0); }

    SegmentedControl segment;
    juce::ParameterAttachment syncAttachment, divisionAttachment;
    bool syncOn = false;
    int division = 2;
};
}

//==============================================================================
class DarthPhaserEditor::Content final : public juce::Component
{
public:
    explicit Content (DarthPhaserProcessor& p)
        : proc (p), state (p.getValueTreeState()),
          modulation ("MODULATION"), phaseEngine ("PHASE ENGINE"),
          lfoPanel ("LFO / SWEEP", false), stereoPanel ("STEREO"), outputPanel ("OUTPUT"),
          speed    (paramRef (state, ParamID::rate),        "SPEED",    Theme::amber, true),
          depth    (paramRef (state, ParamID::depth),       "DEPTH",    Theme::amber),
          stereo   (paramRef (state, ParamID::stereoPhase), "PHASE",    Theme::saber),
          centre   (paramRef (state, ParamID::centre),      "CENTER",   Theme::amber),
          spread   (paramRef (state, ParamID::spread),      "SPREAD",   Theme::amber),
          feedback (paramRef (state, ParamID::feedback),    "FEEDBACK", Theme::saber),
          damping  (paramRef (state, ParamID::damping),     "DAMP",     Theme::amber),
          mix      (paramRef (state, ParamID::mix),         "MIX",      Theme::amber),
          level    (paramRef (state, ParamID::level),       "LEVEL",    Theme::amber),
          waveform (paramRef (state, ParamID::waveform), { "SINE", "TRI", "SQR", "RND" }),
          stages   (paramRef (state, ParamID::stages),   { "2", "4", "6", "8", "12" }),
          sync     (paramRef (state, ParamID::sync), paramRef (state, ParamID::division)),
          waveformCaption ("WAVEFORM"), stagesCaption ("STAGES"),
          syncCaption ("SYNC"), offsetCaption ("L / R OFFSET"),
          scope (p),
          offsetDisplay (paramRef (state, ParamID::stereoPhase)),
          meter ([&p] (int ch) { return p.getOutputPeak (ch); }),
          presets (p.getPresetManager()),
          newPreset   (IconButton::Icon::plus,  "New preset",    Theme::amber),
          savePreset  (IconButton::Icon::save,  "Save preset",   Theme::text.withAlpha (0.75f)),
          deletePreset(IconButton::Icon::trash, "Delete preset", Theme::text.withAlpha (0.75f)),
          power (paramRef (state, ParamID::bypass))
    {
        if (auto xml = juce::parseXML (helmetSvg))
            helmet = juce::Drawable::createFromSVG (*xml);

        for (auto* panel : { &modulation, &phaseEngine, &lfoPanel, &stereoPanel, &outputPanel })
            addAndMakeVisible (panel);

        addAndMakeVisible (presets);
        addAndMakeVisible (newPreset);
        addAndMakeVisible (savePreset);
        addAndMakeVisible (deletePreset);
        addAndMakeVisible (power);

        modulation.addAndMakeVisible (speed);
        modulation.addAndMakeVisible (depth);
        modulation.addAndMakeVisible (waveformCaption);
        modulation.addAndMakeVisible (waveform);

        phaseEngine.addAndMakeVisible (stagesCaption);
        phaseEngine.addAndMakeVisible (stages);
        phaseEngine.addAndMakeVisible (centre);
        phaseEngine.addAndMakeVisible (spread);
        phaseEngine.addAndMakeVisible (feedback);
        phaseEngine.addAndMakeVisible (damping);

        lfoPanel.addAndMakeVisible (syncCaption);
        lfoPanel.addAndMakeVisible (sync);
        lfoPanel.addAndMakeVisible (scope);

        stereoPanel.addAndMakeVisible (stereo);
        stereoPanel.addAndMakeVisible (offsetCaption);
        stereoPanel.addAndMakeVisible (offsetDisplay);

        outputPanel.addAndMakeVisible (mix);
        outputPanel.addAndMakeVisible (level);
        outputPanel.addAndMakeVisible (meter);

        newPreset.onClick    = [this] { promptForName(); };
        savePreset.onClick   = [this] { proc.getPresetManager().overwriteCurrent(); };
        deletePreset.onClick = [this] { confirmDelete(); };

        proc.getPresetManager().onChange = [this] { presetChanged(); };
        presetChanged();

        setSize (DarthPhaserEditor::designWidth, DarthPhaserEditor::designHeight);
    }

    ~Content() override
    {
        proc.getPresetManager().onChange = nullptr;
    }

    void resized() override
    {
        constexpr int pad = 13, gap = 9;

        auto area = getLocalBounds().reduced (pad);

        header = area.removeFromTop (70);
        area.removeFromTop (gap);

        auto rowA = area.removeFromTop (216);
        area.removeFromTop (gap);

        footer = area.removeFromBottom (14);
        area.removeFromBottom (4);

        auto rowB = area;

        modulation.setBounds (rowA.removeFromLeft (330));
        rowA.removeFromLeft (gap);
        phaseEngine.setBounds (rowA);

        lfoPanel.setBounds (rowB.removeFromLeft (640));
        rowB.removeFromLeft (gap);

        stereoPanel.setBounds (rowB.removeFromTop (118));
        rowB.removeFromTop (gap);
        outputPanel.setBounds (rowB);

        // ---- header ----
        auto head = header.reduced (15, 0);
        power.setBounds (head.removeFromRight (54).withSizeKeepingCentre (54, 40));
        head.removeFromRight (10);

        auto icons = head.removeFromRight (88).withSizeKeepingCentre (88, 26);
        newPreset.setBounds (icons.removeFromLeft (26));
        icons.removeFromLeft (5);
        savePreset.setBounds (icons.removeFromLeft (26));
        icons.removeFromLeft (5);
        deletePreset.setBounds (icons.removeFromLeft (26));

        head.removeFromRight (9);
        presets.setBounds (head.removeFromRight (240).withSizeKeepingCentre (240, 30));

        // ---- modulation ----
        speed.setBounds (18, 36, 108, 142);
        depth.setBounds (190, 32, 72, 92);
        waveformCaption.setBounds (148, 136, 170, 12);
        waveform.setBounds (148, 152, 166, 26);

        // ---- phase engine ----
        const auto peWidth = phaseEngine.getWidth();
        stagesCaption.setBounds (16, 34, 120, 12);
        stages.setBounds (16, 50, peWidth - 32, 26);

        constexpr int knobW = 72;
        const auto span = peWidth - 32;
        const auto step = (span - 4 * knobW) / 3;

        centre  .setBounds (16 + 0 * (knobW + step), 96, knobW, 92);
        spread  .setBounds (16 + 1 * (knobW + step), 96, knobW, 92);
        feedback.setBounds (16 + 2 * (knobW + step), 96, knobW, 92);
        damping .setBounds (16 + 3 * (knobW + step), 96, knobW, 92);

        // ---- lfo ----
        auto lfoHeader = lfoPanel.getHeaderBounds().reduced (11, 2);
        sync.setBounds (lfoHeader.removeFromRight (198).withSizeKeepingCentre (198, 19));
        lfoHeader.removeFromRight (8);
        syncCaption.setBounds (lfoHeader.removeFromRight (36));

        scope.setBounds (lfoPanel.getContentBounds().reduced (12, 11));

        // ---- stereo ----
        auto stereoContent = stereoPanel.getContentBounds();
        stereo.setBounds (stereoContent.getX() + 14, stereoContent.getY() + 4, 68, 86);
        offsetCaption.setBounds (stereoContent.getX() + 94, stereoContent.getY() + 7, 150, 12);
        offsetDisplay.setBounds (stereoContent.getX() + 94, stereoContent.getY() + 23,
                                 stereoContent.getRight() - stereoContent.getX() - 110, 52);

        // ---- output ----
        auto out = outputPanel.getContentBounds().reduced (16, 0);
        const auto row = juce::Rectangle<int> (out.getX(), out.getY() + (out.getHeight() - 92) / 2,
                                               out.getWidth(), 92);

        constexpr int groupWidth = 72 + 10 + 72 + 22 + 22;
        const auto groupX = row.getX() + (row.getWidth() - groupWidth) / 2;

        mix.setBounds (groupX, row.getY(), 72, 92);
        level.setBounds (groupX + 82, row.getY(), 72, 92);
        meter.setBounds (groupX + 176, row.getY() + 4, 22, 72);
        meterLabel = { groupX + 176, row.getY() + 80, 22, 10 };
    }

    void paint (juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();

        g.setColour (Theme::background);
        g.fillAll();

        // crimson backlight behind the whole unit
        g.setGradientFill (juce::ColourGradient (Theme::saber.withAlpha (0.13f),
                                                 bounds.getCentreX(), -bounds.getHeight() * 0.25f,
                                                 juce::Colours::transparentBlack,
                                                 bounds.getCentreX(), bounds.getHeight() * 0.62f, true));
        g.fillRect (bounds);

        g.setGradientFill (juce::ColourGradient (Theme::amber.withAlpha (0.07f),
                                                 bounds.getCentreX(), bounds.getBottom(),
                                                 juce::Colours::transparentBlack,
                                                 bounds.getCentreX(), bounds.getHeight() * 0.55f, true));
        g.fillRect (bounds);

        // ---- header plate ----
        const auto plate = header.toFloat();
        g.setGradientFill ({ Theme::panelTop, plate.getX(), plate.getY(),
                             Theme::panelBottom, plate.getX(), plate.getBottom(), false });
        g.fillRoundedRectangle (plate, 5.0f);
        g.setColour (Theme::panelEdge);
        g.drawRoundedRectangle (plate.reduced (0.5f), 5.0f, 1.0f);
        g.setColour (Theme::panelHilite);
        g.drawHorizontalLine (static_cast<int> (plate.getY() + 1.0f), plate.getX() + 3.0f, plate.getRight() - 3.0f);

        auto text = header.reduced (15, 0);
        auto markArea = text.removeFromLeft (46).withSizeKeepingCentre (46, 46);

        if (helmet != nullptr)
            helmet->drawWithin (g, markArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);

        text.removeFromLeft (14);

        auto titleArea = text.removeFromLeft (420);
        auto subtitleArea = titleArea.removeFromBottom (18);

        g.setColour (Theme::text);
        g.setFont (Theme::displayFont (30.0f, 0.17f));
        g.drawText ("DARTH PHASER", titleArea.withTrimmedTop (8), juce::Justification::bottomLeft, false);

        g.setColour (Theme::textDim);
        g.setFont (Theme::label (9.0f, true, 0.42f));
        g.drawText ("STEREO PHASE SHIFTER", subtitleArea, juce::Justification::topLeft, false);

        // ---- meter caption + footer ----
        g.setColour (Theme::textDim);
        g.setFont (Theme::label (8.0f, true, 0.18f));
        g.drawText ("L   R", meterLabel, juce::Justification::centred, false);

        g.setColour (juce::Colour (0xff4e4c48));
        g.setFont (Theme::label (8.0f, true, 0.3f));
        g.drawText ("ROZAN AUDIO", footer, juce::Justification::centredLeft, false);

        auto right = footer;
        auto versionArea = right.removeFromRight (52);
        right.removeFromRight (8);

        // juce::String (const char*) decodes as ASCII, so the middle dot needs the UTF-8 pointer
        g.drawText (juce::String (juce::CharPointer_UTF8 ("VST3  \xc2\xb7  STANDALONE")),
                    right.removeFromRight (160), juce::Justification::centredRight, false);
        g.setColour (Theme::amber.withAlpha (0.85f));
        g.drawText ("v" JucePlugin_VersionString, versionArea, juce::Justification::centredRight, false);
    }

private:
    void presetChanged()
    {
        const auto factory = proc.getPresetManager().currentIsFactory();
        savePreset.setEnabled (! factory);
        deletePreset.setEnabled (! factory);
        presets.repaint();
    }

    void promptForName()
    {
        auto* window = new juce::AlertWindow ("New Preset", "Name this preset",
                                              juce::MessageBoxIconType::NoIcon);
        window->addTextEditor ("name", proc.getPresetManager().getCurrentName());
        window->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
        window->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

        juce::Component::SafePointer<Content> safeThis { this };
        juce::Component::SafePointer<juce::AlertWindow> safeWindow { window };

        window->enterModalState (true, juce::ModalCallbackFunction::create (
            [safeThis, safeWindow] (int result)
            {
                if (result != 1 || safeThis == nullptr || safeWindow == nullptr)
                    return;

                safeThis->proc.getPresetManager().saveAs (safeWindow->getTextEditorContents ("name"));
            }), true);
    }

    void confirmDelete()
    {
        auto& manager = proc.getPresetManager();

        if (manager.currentIsFactory())
            return;

        juce::Component::SafePointer<Content> safeThis { this };

        juce::AlertWindow::showOkCancelBox (
            juce::MessageBoxIconType::WarningIcon, "Delete Preset",
            "Move \"" + manager.getCurrentName() + "\" to the trash?",
            "Delete", "Cancel", nullptr,
            juce::ModalCallbackFunction::create (
                [safeThis] (int result)
                {
                    if (result == 1 && safeThis != nullptr)
                        safeThis->proc.getPresetManager().deleteCurrent();
                }));
    }

    DarthPhaserProcessor& proc;
    juce::AudioProcessorValueTreeState& state;

    PanelSection modulation, phaseEngine, lfoPanel, stereoPanel, outputPanel;
    Knob speed, depth, stereo, centre, spread, feedback, damping, mix, level;
    ParameterSegment waveform, stages;
    SyncSelector sync;
    Caption waveformCaption, stagesCaption, syncCaption, offsetCaption;
    LfoScope scope;
    StereoOffsetDisplay offsetDisplay;
    LevelMeter meter;
    PresetBar presets;
    IconButton newPreset, savePreset, deletePreset;
    PowerLed power;

    std::unique_ptr<juce::Drawable> helmet;
    juce::Rectangle<int> header, footer, meterLabel;
};

//==============================================================================
DarthPhaserEditor::DarthPhaserEditor (DarthPhaserProcessor& p)
    : AudioProcessorEditor (&p)
{
    setLookAndFeel (&lookAndFeel);

    content = std::make_unique<Content> (p);
    addAndMakeVisible (*content);

    constrainer.setFixedAspectRatio (static_cast<double> (designWidth) / static_cast<double> (designHeight));
    constrainer.setSizeLimits (designWidth / 2, designHeight / 2, designWidth * 2, designHeight * 2);
    setConstrainer (&constrainer);

    setResizable (true, true);
    setSize (designWidth, designHeight);
}

DarthPhaserEditor::~DarthPhaserEditor()
{
    setConstrainer (nullptr);
    setLookAndFeel (nullptr);
}

void DarthPhaserEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::background);
}

void DarthPhaserEditor::resized()
{
    const auto scale = static_cast<float> (getWidth()) / static_cast<float> (designWidth);

    content->setBounds (0, 0, designWidth, designHeight);
    content->setTransform (juce::AffineTransform::scale (scale));
}
