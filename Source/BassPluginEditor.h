#pragma once

#include <array>
#include <memory>

#include <juce_gui_extra/juce_gui_extra.h>

#include "BassPluginProcessor.h"

class AphexBassAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AphexBassAudioProcessorEditor(AphexBassAudioProcessor&);
    ~AphexBassAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    struct LookAndFeel final : juce::LookAndFeel_V4
    {
        LookAndFeel();
    };

    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void setParameterValue(const juce::String& paramId, float plainValue);
    void applyPreset(int presetIndex);

    AphexBassAudioProcessor& audioProcessor;
    LookAndFeel lookAndFeel;

    std::array<juce::Slider, 22> sliders;
    std::array<juce::Label, 22> labels;
    std::array<std::unique_ptr<SliderAttachment>, 22> attachments;

    juce::Label titleLabel;
    juce::Label infoLabel;
    juce::Label presetLabel;
    juce::ComboBox presetBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AphexBassAudioProcessorEditor)
};
