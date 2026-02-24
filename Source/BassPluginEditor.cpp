#include "BassPluginEditor.h"

namespace
{
const juce::Colour bg { 0xff020401 };
const juce::Colour panel { 0xff050a03 };
const juce::Colour phosphor { 0xff8dff67 };
const juce::Colour phosphorHot { 0xffb9ff9e };
const juce::Colour phosphorDim { 0xff4a9535 };
const juce::Colour textMain { 0xffc9ffb8 };

constexpr std::array<const char*, 22> parameterIds {
    "output", "tune", "glide", "oscMix", "sub", "fmAmt", "fmRatio", "fold", "drive", "noise",
    "cutoff", "resonance", "envAmt", "lfoRate", "lfoToCutoff", "stereo", "attack", "decay", "sustain", "release",
    "monoLegato", "accent"
};

constexpr std::array<const char*, 22> sliderNames {
    "Output", "Tune", "Glide", "Osc", "Sub", "FM Amt", "FM Ratio", "Fold", "Drive", "Noise",
    "Cutoff", "Res", "Env Amt", "LFO Rate", "LFO -> F", "Stereo", "Attack", "Decay", "Sustain", "Release",
    "Legato", "Accent"
};

struct PresetData
{
    const char* name = "";
    std::array<float, 22> values {};
};

constexpr std::array<PresetData, 3> presets {{
    {
        "drukqs metallic sub",
        { -8.5f, 0.0f, 0.020f, 0.84f, 0.68f, 0.42f, 2.75f, 0.50f, 0.56f, 0.03f, 240.0f, 0.62f, 0.76f, 3.2f, 0.20f, 0.18f, 0.002f, 0.14f, 0.58f, 0.18f, 1.0f, 0.66f }
    },
    {
        "syro rubber bass",
        { -9.5f, -12.0f, 0.065f, 0.34f, 0.80f, 0.22f, 1.35f, 0.22f, 0.46f, 0.06f, 180.0f, 0.34f, 0.68f, 1.1f, 0.16f, 0.12f, 0.006f, 0.22f, 0.72f, 0.30f, 1.0f, 0.45f }
    },
    {
        "ventolin broken acid bass",
        { -11.0f, 7.0f, 0.012f, 0.70f, 0.30f, 0.76f, 4.40f, 0.72f, 0.78f, 0.10f, 410.0f, 0.84f, 0.92f, 6.8f, 0.54f, 0.34f, 0.001f, 0.11f, 0.36f, 0.15f, 0.0f, 0.92f }
    }
}};
}

AphexBassAudioProcessorEditor::LookAndFeel::LookAndFeel()
{
    setDefaultSansSerifTypefaceName("Menlo");
}

AphexBassAudioProcessorEditor::AphexBassAudioProcessorEditor(AphexBassAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(1080, 430);

    titleLabel.setText("D-BASS", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, phosphorHot);
    titleLabel.setFont(juce::Font(juce::FontOptions(20.0f).withStyle("Bold")));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    infoLabel.setText("MONO BASS SYNTH  |  FM  |  FOLD  |  FILTER", juce::dontSendNotification);
    infoLabel.setColour(juce::Label::textColourId, phosphorDim);
    infoLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    infoLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(infoLabel);

    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, phosphor);
    presetLabel.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    presetLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(presetLabel);

    presetBox.setColour(juce::ComboBox::backgroundColourId, panel);
    presetBox.setColour(juce::ComboBox::outlineColourId, phosphorDim);
    presetBox.setColour(juce::ComboBox::textColourId, phosphor);
    presetBox.setColour(juce::ComboBox::arrowColourId, phosphor);
    for (size_t i = 0; i < presets.size(); ++i)
        presetBox.addItem(presets[i].name, static_cast<int>(i + 1));
    presetBox.onChange = [this]
    {
        const int selected = presetBox.getSelectedId();
        if (selected > 0)
            applyPreset(selected - 1);
    };
    addAndMakeVisible(presetBox);

    auto& apvts = audioProcessor.getAPVTS();

    for (size_t i = 0; i < sliders.size(); ++i)
    {
        configureSlider(sliders[i], labels[i], sliderNames[i]);
        attachments[i] = std::make_unique<SliderAttachment>(apvts, parameterIds[i], sliders[i]);
    }

    presetBox.setSelectedId(1, juce::sendNotificationSync);
}

AphexBassAudioProcessorEditor::~AphexBassAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AphexBassAudioProcessorEditor::configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
    slider.setColour(juce::Slider::trackColourId, phosphorDim);
    slider.setColour(juce::Slider::thumbColourId, phosphor);
    slider.setColour(juce::Slider::backgroundColourId, panel.brighter(0.02f));
    slider.setColour(juce::Slider::textBoxTextColourId, phosphor);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, panel);
    slider.setColour(juce::Slider::textBoxOutlineColourId, phosphorDim);
    addAndMakeVisible(slider);

    label.setText(text.toUpperCase(), juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, textMain);
    label.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void AphexBassAudioProcessorEditor::setParameterValue(const juce::String& paramId, float plainValue)
{
    auto* parameter = audioProcessor.getAPVTS().getParameter(paramId);
    auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter);
    if (ranged == nullptr)
        return;

    const float normalized = ranged->convertTo0to1(plainValue);
    ranged->beginChangeGesture();
    ranged->setValueNotifyingHost(normalized);
    ranged->endChangeGesture();
}

void AphexBassAudioProcessorEditor::applyPreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= static_cast<int>(presets.size()))
        return;

    const auto& preset = presets[static_cast<size_t>(presetIndex)];
    for (size_t i = 0; i < parameterIds.size(); ++i)
        setParameterValue(parameterIds[i], preset.values[i]);
}

void AphexBassAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);

    auto panelArea = getLocalBounds().reduced(10);
    g.setColour(panel);
    g.fillRect(panelArea);
    g.setColour(phosphorDim.withAlpha(0.85f));
    g.drawRect(panelArea, 1);
    g.setColour(phosphorDim.withAlpha(0.35f));
    g.drawRect(panelArea.reduced(3), 1);

    g.setColour(phosphorDim.withAlpha(0.14f));
    for (int y = panelArea.getY(); y < panelArea.getBottom(); y += 3)
        g.drawHorizontalLine(y, static_cast<float>(panelArea.getX()), static_cast<float>(panelArea.getRight()));

    g.setColour(phosphorDim.withAlpha(0.55f));
    const float separatorY = static_cast<float>(panelArea.getY() + 44);
    g.drawLine(static_cast<float>(panelArea.getX() + 1), separatorY, static_cast<float>(panelArea.getRight() - 1), separatorY, 1.0f);

    auto content = getLocalBounds().reduced(14);
    content.removeFromTop(54);
    const int columns = 11;
    const int rows = 2;
    const int cellGap = 5;
    const int totalGapX = cellGap * (columns - 1);
    const int totalGapY = cellGap * (rows - 1);
    const int cellW = (content.getWidth() - totalGapX) / columns;
    const int cellH = (content.getHeight() - totalGapY) / rows;

    auto makeSectionRect = [=](int row, int colStart, int colEnd)
    {
        return juce::Rectangle<int>(
            content.getX() + colStart * (cellW + cellGap),
            content.getY() + row * (cellH + cellGap),
            ((colEnd - colStart + 1) * cellW) + ((colEnd - colStart) * cellGap),
            cellH).reduced(1);
    };

    g.setColour(phosphorDim.withAlpha(0.22f));
    const auto topA = makeSectionRect(0, 0, 2);
    const auto topB = makeSectionRect(0, 3, 7);
    const auto topC = makeSectionRect(0, 8, 10);
    const auto bottomA = makeSectionRect(1, 0, 4);
    const auto bottomB = makeSectionRect(1, 5, 8);
    const auto bottomC = makeSectionRect(1, 9, 10);
    g.drawRect(topA, 1);
    g.drawRect(topB, 1);
    g.drawRect(topC, 1);
    g.drawRect(bottomA, 1);
    g.drawRect(bottomB, 1);
    g.drawRect(bottomC, 1);

    const auto statusBox = panelArea.removeFromTop(16).removeFromRight(84).reduced(4, 2);
    g.setColour(phosphorDim.withAlpha(0.2f));
    g.fillRect(statusBox);
    g.setColour(phosphorDim.withAlpha(0.7f));
    g.drawRect(statusBox, 1);
    g.setColour(phosphorHot.withAlpha(0.9f));
    g.setFont(juce::Font(juce::FontOptions(9.0f).withStyle("Bold")));
    g.drawText("READY", statusBox, juce::Justification::centred, false);
}

void AphexBassAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(14);

    auto header = bounds.removeFromTop(48);
    titleLabel.setBounds(header.removeFromLeft(124));
    infoLabel.setBounds(header.removeFromLeft(386));
    presetLabel.setBounds(header.removeFromLeft(48));
    presetBox.setBounds(header.removeFromLeft(320).reduced(0, 4));

    bounds.removeFromTop(6);

    const int columns = 11;
    const int rows = 2;
    const int cellGap = 5;
    const int totalGapX = cellGap * (columns - 1);
    const int totalGapY = cellGap * (rows - 1);
    const int cellW = (bounds.getWidth() - totalGapX) / columns;
    const int cellH = (bounds.getHeight() - totalGapY) / rows;

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < columns; ++col)
        {
            const int idx = row * columns + col;
            if (idx >= static_cast<int>(sliders.size()))
                break;

            auto cell = juce::Rectangle<int>(
                bounds.getX() + col * (cellW + cellGap),
                bounds.getY() + row * (cellH + cellGap),
                cellW,
                cellH);

            auto top = cell.removeFromTop(18);
            labels[static_cast<size_t>(idx)].setBounds(top);
            cell.removeFromTop(2);
            sliders[static_cast<size_t>(idx)].setBounds(cell.reduced(2));
        }
    }
}
