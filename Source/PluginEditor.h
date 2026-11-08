#pragma once

#include "PluginProcessor.h"

namespace EclipseColours
{
    const juce::Colour bgTop    { 0xff1a0b2e };
    const juce::Colour bgBottom { 0xff07020f };
    const juce::Colour purple   { 0xffa855f7 };
    const juce::Colour violet   { 0xff7c3aed };
    const juce::Colour glow     { 0xffd8b4fe };
    const juce::Colour track    { 0xff2e1a4a };
    const juce::Colour text     { 0xffe9d5ff };
}

class EclipseLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;
};

// Live picture of what the clipper is doing to the waveform.
class CurveView : public juce::Component, private juce::Timer
{
public:
    explicit CurveView (juce::AudioProcessorValueTreeState& s);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }
    juce::AudioProcessorValueTreeState& apvts;
};

// Soft | Hard pill switch.
class ModeSwitch : public juce::Component
{
public:
    explicit ModeSwitch (juce::AudioProcessorValueTreeState& s);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    juce::RangedAudioParameter* param;
    juce::ParameterAttachment attachment;
    bool hard = false;
};

class EclipseAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit EclipseAudioProcessorEditor (EclipseAudioProcessor&);
    ~EclipseAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    EclipseAudioProcessor& processor;
    EclipseLookAndFeel laf;

    juce::Slider clipKnob, outKnob;
    juce::Label clipLabel, outLabel;
    CurveView curve;
    ModeSwitch mode;

    juce::AudioProcessorValueTreeState::SliderAttachment clipAttach, outAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EclipseAudioProcessorEditor)
};
