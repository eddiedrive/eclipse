#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr auto kDrive  = "drive";
    constexpr auto kHard   = "hard";
    constexpr auto kOutput = "output";
}

juce::AudioProcessorValueTreeState::ParameterLayout EclipseAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { kDrive, 1 }, "Clip Amount",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f,
        AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { kHard, 1 }, "Hard Clip", false));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { kOutput, 1 }, "Output",
        NormalisableRange<float> (-24.0f, 0.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));

    return layout;
}

EclipseAudioProcessor::EclipseAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    driveParam  = apvts.getRawParameterValue (kDrive);
    hardParam   = apvts.getRawParameterValue (kHard);
    outputParam = apvts.getRawParameterValue (kOutput);
}

float EclipseAudioProcessor::driveToGain (float amountPercent)
{
    // 0% = unity, 100% = +36 dB of push into the clipper.
    return juce::Decibels::decibelsToGain (juce::jlimit (0.0f, 100.0f, amountPercent) * 0.36f);
}

float EclipseAudioProcessor::shape (float x, float gain, bool hard)
{
    const float v = x * gain;
    return hard ? juce::jlimit (-1.0f, 1.0f, v) : std::tanh (v);
}

void EclipseAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    oversampling.initProcessing ((size_t) samplesPerBlock);
    oversampling.reset();
    setLatencySamples ((int) oversampling.getLatencyInSamples());

    const double osRate = sampleRate * (double) oversampling.getOversamplingFactor();
    smoothedGain.reset (osRate, 0.02);
    smoothedOutput.reset (osRate, 0.02);
    smoothedGain.setCurrentAndTargetValue (driveToGain (driveParam->load()));
    smoothedOutput.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (outputParam->load()));
}

bool EclipseAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
           && out == layouts.getMainInputChannelSet();
}

void EclipseAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    smoothedGain.setTargetValue (driveToGain (driveParam->load()));
    smoothedOutput.setTargetValue (juce::Decibels::decibelsToGain (outputParam->load()));
    const bool hard = hardParam->load() > 0.5f;

    juce::dsp::AudioBlock<float> block (buffer);
    auto upBlock = oversampling.processSamplesUp (block);

    const auto numCh      = upBlock.getNumChannels();
    const auto numSamples = upBlock.getNumSamples();

    for (size_t i = 0; i < numSamples; ++i)
    {
        const float g = smoothedGain.getNextValue();
        const float o = smoothedOutput.getNextValue();

        for (size_t ch = 0; ch < numCh; ++ch)
        {
            auto* d = upBlock.getChannelPointer (ch);
            d[i] = shape (d[i], g, hard) * o;
        }
    }

    oversampling.processSamplesDown (block);
}

juce::AudioProcessorEditor* EclipseAudioProcessor::createEditor()
{
    return new EclipseAudioProcessorEditor (*this);
}

void EclipseAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void EclipseAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EclipseAudioProcessor();
}
