#pragma once

#include <array>
#include <atomic>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class AphexBassAudioProcessor final : public juce::AudioProcessor
{
public:
    AphexBassAudioProcessor();
    ~AphexBassAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    void retargetFrequencyFromHeldNotes();

    static float softClip(float x);
    static float waveFold(float x, float amount);

    double currentSampleRate = 44100.0;

    juce::AudioProcessorValueTreeState parameters;

    juce::ADSR ampEnv;
    juce::ADSR filterEnv;
    juce::ADSR::Parameters ampEnvParams;
    juce::ADSR::Parameters filterEnvParams;

    juce::dsp::StateVariableTPTFilter<float> filterL;
    juce::dsp::StateVariableTPTFilter<float> filterR;

    float phaseMain = 0.0f;
    float phaseSub = 0.0f;
    float phaseFm = 0.0f;
    float lfoPhase = 0.0f;

    float currentFrequency = 55.0f;
    float targetFrequency = 55.0f;
    float lastVelocity = 1.0f;

    std::vector<int> heldNotes;

    std::atomic<float>* outputParam = nullptr;
    std::atomic<float>* tuneParam = nullptr;
    std::atomic<float>* glideParam = nullptr;
    std::atomic<float>* oscMixParam = nullptr;
    std::atomic<float>* subParam = nullptr;
    std::atomic<float>* fmAmtParam = nullptr;
    std::atomic<float>* fmRatioParam = nullptr;
    std::atomic<float>* foldParam = nullptr;
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* noiseParam = nullptr;
    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* resonanceParam = nullptr;
    std::atomic<float>* envAmtParam = nullptr;
    std::atomic<float>* lfoRateParam = nullptr;
    std::atomic<float>* lfoToCutoffParam = nullptr;
    std::atomic<float>* stereoParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* monoLegatoParam = nullptr;
    std::atomic<float>* accentParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AphexBassAudioProcessor)
};
