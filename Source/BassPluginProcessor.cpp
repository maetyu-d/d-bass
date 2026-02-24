#include "BassPluginProcessor.h"
#include "BassPluginEditor.h"

#include <algorithm>
#include <cmath>
#include <memory>

namespace
{
constexpr float twoPi = juce::MathConstants<float>::twoPi;

float midiNoteToHz(int note)
{
    return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
}

float expSlewCoefficient(float timeSeconds, float sampleRate)
{
    const float t = juce::jmax(0.0001f, timeSeconds);
    return std::exp(-1.0f / (t * sampleRate));
}

float readParam(const std::atomic<float>* p, float fallback)
{
    return p != nullptr ? p->load() : fallback;
}
}

AphexBassAudioProcessor::AphexBassAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    outputParam = parameters.getRawParameterValue("output");
    tuneParam = parameters.getRawParameterValue("tune");
    glideParam = parameters.getRawParameterValue("glide");
    oscMixParam = parameters.getRawParameterValue("oscMix");
    subParam = parameters.getRawParameterValue("sub");
    fmAmtParam = parameters.getRawParameterValue("fmAmt");
    fmRatioParam = parameters.getRawParameterValue("fmRatio");
    foldParam = parameters.getRawParameterValue("fold");
    driveParam = parameters.getRawParameterValue("drive");
    noiseParam = parameters.getRawParameterValue("noise");
    cutoffParam = parameters.getRawParameterValue("cutoff");
    resonanceParam = parameters.getRawParameterValue("resonance");
    envAmtParam = parameters.getRawParameterValue("envAmt");
    lfoRateParam = parameters.getRawParameterValue("lfoRate");
    lfoToCutoffParam = parameters.getRawParameterValue("lfoToCutoff");
    stereoParam = parameters.getRawParameterValue("stereo");
    attackParam = parameters.getRawParameterValue("attack");
    decayParam = parameters.getRawParameterValue("decay");
    sustainParam = parameters.getRawParameterValue("sustain");
    releaseParam = parameters.getRawParameterValue("release");
    monoLegatoParam = parameters.getRawParameterValue("monoLegato");
    accentParam = parameters.getRawParameterValue("accent");
}

juce::AudioProcessorValueTreeState::ParameterLayout AphexBassAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("output", "Output", juce::NormalisableRange<float>(-24.0f, 6.0f, 0.01f), -8.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("tune", "Tune", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("glide", "Glide", juce::NormalisableRange<float>(0.0f, 0.35f, 0.0001f, 0.4f), 0.025f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("oscMix", "Osc Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.72f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sub", "Sub", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.62f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fmAmt", "FM Amount", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.28f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fmRatio", "FM Ratio", juce::NormalisableRange<float>(0.25f, 8.0f, 0.001f, 0.35f), 2.0f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fold", "Fold", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.36f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.45f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("noise", "Noise", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.07f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", juce::NormalisableRange<float>(30.0f, 14000.0f, 0.01f, 0.23f), 220.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance", juce::NormalisableRange<float>(0.05f, 0.95f, 0.001f), 0.28f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("envAmt", "Env Amount", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.72f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("lfoRate", "LFO Rate", juce::NormalisableRange<float>(0.05f, 24.0f, 0.001f, 0.35f), 2.8f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("lfoToCutoff", "LFO->Cutoff", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.22f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("stereo", "Stereo", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.001f, 0.25f, 0.0001f, 0.4f), 0.003f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.02f, 1.2f, 0.0001f, 0.35f), 0.18f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.66f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.01f, 2.5f, 0.0001f, 0.35f), 0.21f));
    layout.push_back(std::make_unique<juce::AudioParameterBool>("monoLegato", "Mono Legato", true));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("accent", "Accent", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

    return { layout.begin(), layout.end() };
}

void AphexBassAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    currentSampleRate = juce::jmax(8000.0, sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(juce::jmax(64, samplesPerBlock));
    spec.numChannels = 1;

    filterL.prepare(spec);
    filterR.prepare(spec);
    filterL.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filterR.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filterL.reset();
    filterR.reset();

    ampEnv.reset();
    filterEnv.reset();
    ampEnv.setSampleRate(currentSampleRate);
    filterEnv.setSampleRate(currentSampleRate);

    phaseMain = phaseSub = phaseFm = lfoPhase = 0.0f;
    currentFrequency = targetFrequency = 55.0f;
    bassBloomStateL = 0.0f;
    bassBloomStateR = 0.0f;
    heldNotes.clear();
}

void AphexBassAudioProcessor::releaseResources()
{
}

bool AphexBassAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AphexBassAudioProcessor::noteOn(int midiNote, float velocity)
{
    const bool monoLegato = readParam(monoLegatoParam, 1.0f) >= 0.5f;
    const bool hadHeldNotes = !heldNotes.empty();

    heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), midiNote), heldNotes.end());
    heldNotes.push_back(midiNote);

    lastVelocity = juce::jlimit(0.0f, 1.0f, velocity);

    const float tuneSemi = readParam(tuneParam, 0.0f);
    targetFrequency = midiNoteToHz(midiNote) * std::pow(2.0f, tuneSemi / 12.0f);

    if (!ampEnv.isActive())
    {
        currentFrequency = targetFrequency;
        ampEnv.noteOn();
        filterEnv.noteOn();
        return;
    }

    if (!monoLegato || !hadHeldNotes)
    {
        ampEnv.noteOn();
        filterEnv.noteOn();
    }
}

void AphexBassAudioProcessor::noteOff(int midiNote)
{
    heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), midiNote), heldNotes.end());

    if (heldNotes.empty())
    {
        ampEnv.noteOff();
        filterEnv.noteOff();
        return;
    }

    retargetFrequencyFromHeldNotes();
}

void AphexBassAudioProcessor::retargetFrequencyFromHeldNotes()
{
    if (heldNotes.empty())
        return;

    const int activeNote = heldNotes.back();
    const float tuneSemi = readParam(tuneParam, 0.0f);
    targetFrequency = midiNoteToHz(activeNote) * std::pow(2.0f, tuneSemi / 12.0f);
}

float AphexBassAudioProcessor::softClip(float x)
{
    return std::tanh(x);
}

float AphexBassAudioProcessor::waveFold(float x, float amount)
{
    if (amount <= 0.001f)
        return x;

    const float drive = 1.0f + amount * 4.0f;
    const float folded = std::sin(x * drive * juce::MathConstants<float>::halfPi);
    return juce::jmap(amount, x, folded);
}

void AphexBassAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    ampEnvParams.attack = readParam(attackParam, 0.003f);
    ampEnvParams.decay = readParam(decayParam, 0.18f);
    ampEnvParams.sustain = readParam(sustainParam, 0.66f);
    ampEnvParams.release = readParam(releaseParam, 0.21f);
    ampEnv.setParameters(ampEnvParams);

    filterEnvParams.attack = ampEnvParams.attack * 0.3f;
    filterEnvParams.decay = juce::jmax(0.03f, ampEnvParams.decay * 0.6f);
    filterEnvParams.sustain = juce::jlimit(0.0f, 1.0f, ampEnvParams.sustain * 0.75f);
    filterEnvParams.release = juce::jmax(0.02f, ampEnvParams.release * 0.7f);
    filterEnv.setParameters(filterEnvParams);

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        if (message.isNoteOn())
            noteOn(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            noteOff(message.getNoteNumber());
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            heldNotes.clear();
            ampEnv.noteOff();
            filterEnv.noteOff();
        }
    }

    midiMessages.clear();
    buffer.clear();

    const float oscMix = readParam(oscMixParam, 0.72f);
    const float subMix = readParam(subParam, 0.62f);
    const float fmAmt = readParam(fmAmtParam, 0.28f);
    const float fmRatio = readParam(fmRatioParam, 2.0f);
    const float fold = readParam(foldParam, 0.36f);
    const float drive = readParam(driveParam, 0.45f);
    const float noise = readParam(noiseParam, 0.07f);
    const float cutoff = readParam(cutoffParam, 220.0f);
    const float resonance = readParam(resonanceParam, 0.28f);
    const float envAmt = readParam(envAmtParam, 0.72f);
    const float lfoRate = readParam(lfoRateParam, 2.8f);
    const float lfoToCutoff = readParam(lfoToCutoffParam, 0.22f);
    const float stereo = readParam(stereoParam, 0.25f);
    const float outputDb = readParam(outputParam, -8.0f);
    const float accent = readParam(accentParam, 0.5f);
    const float outputGain = juce::Decibels::decibelsToGain(outputDb);

    const float sampleRate = static_cast<float>(currentSampleRate);
    const float glideTime = readParam(glideParam, 0.025f);
    const float glideCoeff = expSlewCoefficient(glideTime, sampleRate);

    const float lfoIncrement = twoPi * lfoRate / sampleRate;
    const float accentVelocity = juce::jlimit(0.0f, 1.0f, (lastVelocity - 0.55f) * 2.2f);
    const float accentBoost = accent * accentVelocity;
    const float driveGain = 1.0f + 15.0f * drive * (1.0f + 0.5f * accentBoost);
    const float driveTrim = 1.0f / std::sqrt(juce::jmax(1.0f, driveGain));

    juce::Random random;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        currentFrequency = glideCoeff * currentFrequency + (1.0f - glideCoeff) * targetFrequency;
        currentFrequency = juce::jlimit(20.0f, 12000.0f, currentFrequency);

        const float lfo = std::sin(lfoPhase);
        lfoPhase += lfoIncrement;
        if (lfoPhase >= twoPi)
            lfoPhase -= twoPi;

        const float fmOsc = std::sin(phaseFm);
        const float fmHz = fmOsc * (fmAmt * 600.0f);

        const float phaseIncrementMain = twoPi * (currentFrequency + fmHz) / sampleRate;
        const float phaseIncrementSub = twoPi * (currentFrequency * 0.5f) / sampleRate;
        const float phaseIncrementFm = twoPi * (currentFrequency * fmRatio) / sampleRate;

        const float phaseNorm = phaseMain / twoPi;
        const float saw = (2.0f * phaseNorm) - 1.0f;
        const float pulseWidth = juce::jlimit(0.12f, 0.88f, 0.49f + 0.18f * lfo * (0.2f + fmAmt));
        const float pulse = phaseNorm < pulseWidth ? 1.0f : -1.0f;
        const float mainOsc = juce::jmap(oscMix, saw, pulse);

        const float subPure = std::sin(phaseSub);
        const float subSaturated = softClip(subPure * (1.7f + subMix * 0.9f));
        const float subOsc = juce::jmap(0.34f + subMix * 0.5f, subPure, subSaturated);
        const float noiseSig = random.nextFloat() * 2.0f - 1.0f;

        float voice = (mainOsc * (1.0f - subMix * 0.9f)) + (subOsc * (subMix * 1.08f));
        voice += noiseSig * noise;

        voice = waveFold(voice, fold);
        voice = softClip(voice * driveGain) * driveTrim;

        const float ampValue = ampEnv.getNextSample();
        const float filtEnv = filterEnv.getNextSample();

        const float envAmtWithAccent = envAmt + (accentBoost * 0.45f);
        const float cutoffModSemis = envAmtWithAccent * (filtEnv - 0.2f) * 72.0f + lfo * lfoToCutoff * 36.0f;
        const float cutoffL = juce::jlimit(20.0f, 18000.0f, cutoff * std::pow(2.0f, cutoffModSemis / 12.0f));
        const float cutoffR = juce::jlimit(20.0f, 18000.0f, cutoffL * std::pow(2.0f, (stereo * lfo * 4.0f) / 12.0f));

        filterL.setCutoffFrequency(cutoffL);
        filterR.setCutoffFrequency(cutoffR);
        filterL.setResonance(resonance);
        filterR.setResonance(resonance);

        const float velocityGain = (0.25f + 0.75f * lastVelocity) * (1.0f + 0.22f * accentBoost);
        const float monoSignal = voice * ampValue * velocityGain;
        float left = filterL.processSample(0, monoSignal);
        float right = filterR.processSample(0, monoSignal);

        // Add controlled post-filter low-end bloom for a fatter body.
        const float bloomCoeff = 0.030f;
        bassBloomStateL += bloomCoeff * (left - bassBloomStateL);
        bassBloomStateR += bloomCoeff * (right - bassBloomStateR);
        const float bloomAmount = (0.14f + 0.34f * subMix) * (1.0f + 0.24f * drive);
        left += softClip(bassBloomStateL * 2.4f) * bloomAmount;
        right += softClip(bassBloomStateR * 2.4f) * bloomAmount;
        left = softClip(left * 0.9f);
        right = softClip(right * 0.9f);

        if (numChannels > 0)
            buffer.setSample(0, sample, left * outputGain);
        if (numChannels > 1)
            buffer.setSample(1, sample, right * outputGain);

        phaseMain += phaseIncrementMain;
        phaseSub += phaseIncrementSub;
        phaseFm += phaseIncrementFm;

        if (phaseMain >= twoPi)
            phaseMain -= twoPi;
        if (phaseSub >= twoPi)
            phaseSub -= twoPi;
        if (phaseFm >= twoPi)
            phaseFm -= twoPi;
    }
}

void AphexBassAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto state = parameters.copyState(); state.isValid())
    {
        if (const auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
    }
}

void AphexBassAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState == nullptr)
        return;

    if (!xmlState->hasTagName(parameters.state.getType()))
        return;

    parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    retargetFrequencyFromHeldNotes();
}

juce::AudioProcessorEditor* AphexBassAudioProcessor::createEditor()
{
    return new AphexBassAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AphexBassAudioProcessor();
}
