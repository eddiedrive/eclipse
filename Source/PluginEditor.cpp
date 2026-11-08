#include "PluginEditor.h"

using namespace EclipseColours;

//==============================================================================
void EclipseLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                           float pos, float startAngle, float endAngle,
                                           juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto centre  = bounds.getCentre();
    const float angle  = startAngle + pos * (endAngle - startAngle);
    const float arcW   = juce::jmax (4.0f, radius * 0.10f);
    const float arcR   = radius - arcW * 0.5f;

    // outer glow scales with the value
    for (int i = 5; i > 0; --i)
    {
        g.setColour (purple.withAlpha (0.035f * (float) i * (0.3f + pos)));
        g.fillEllipse (bounds.expanded ((float) (6 - i) * 1.5f));
    }

    // track
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (EclipseColours::track);
    g.strokePath (track, juce::PathStrokeType (arcW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // value arc
    if (pos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
        g.setGradientFill (juce::ColourGradient (violet, centre.x - radius, centre.y + radius,
                                                 glow,   centre.x + radius, centre.y - radius, false));
        g.strokePath (val, juce::PathStrokeType (arcW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // knob body
    const float bodyR = radius - arcW * 2.0f;
    juce::Rectangle<float> body (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3b1d63), centre.x, body.getY(),
                                             juce::Colour (0xff12071f), centre.x, body.getBottom(), false));
    g.fillEllipse (body);
    g.setColour (purple.withAlpha (0.45f));
    g.drawEllipse (body, 1.5f);

    // pointer
    juce::Path p;
    const float pl = bodyR * 0.55f, pw = juce::jmax (3.0f, bodyR * 0.07f);
    p.addRoundedRectangle (-pw * 0.5f, -bodyR + 5.0f, pw, pl, pw * 0.5f);
    g.setColour (glow);
    g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

    // value readout in the centre
    g.setColour (text.withAlpha (0.9f));
    g.setFont (juce::FontOptions ((float) bodyR * 0.38f, juce::Font::bold));
    g.drawText (slider.getTextFromValue (slider.getValue()),
                body.withTrimmedTop (bodyR * 0.95f).withHeight (bodyR * 0.6f),
                juce::Justification::centred);
}

//==============================================================================
CurveView::CurveView (juce::AudioProcessorValueTreeState& s) : apvts (s)
{
    startTimerHz (30);
}

void CurveView::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0d0518));
    g.fillRoundedRectangle (r, 12.0f);
    g.setColour (EclipseColours::track);
    g.drawRoundedRectangle (r, 12.0f, 1.5f);

    auto area = r.reduced (12.0f);
    g.setColour (EclipseColours::track.withAlpha (0.8f));
    g.drawHorizontalLine ((int) area.getCentreY(), area.getX(), area.getRight());
    g.drawVerticalLine ((int) area.getCentreX(), area.getY(), area.getBottom());

    const float gain = EclipseAudioProcessor::driveToGain (*apvts.getRawParameterValue ("drive"));
    const bool hard  = *apvts.getRawParameterValue ("hard") > 0.5f;
    const float out  = juce::Decibels::decibelsToGain ((float) *apvts.getRawParameterValue ("output"));

    juce::Path curve;
    const int n = (int) area.getWidth();
    for (int i = 0; i <= n; ++i)
    {
        const float xin = juce::jmap ((float) i, 0.0f, (float) n, -1.0f, 1.0f);
        const float yo  = juce::jlimit (-1.0f, 1.0f, EclipseAudioProcessor::shape (xin, gain, hard) * out);
        const float px = area.getX() + (float) i;
        const float py = area.getCentreY() - yo * (area.getHeight() * 0.5f);
        if (i == 0) curve.startNewSubPath (px, py); else curve.lineTo (px, py);
    }

    g.setColour (purple.withAlpha (0.25f));
    g.strokePath (curve, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (glow);
    g.strokePath (curve, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

//==============================================================================
ModeSwitch::ModeSwitch (juce::AudioProcessorValueTreeState& s)
    : param (s.getParameter ("hard")),
      attachment (*param, [this] (float v) { hard = v > 0.5f; repaint(); })
{
    attachment.sendInitialUpdate();
}

void ModeSwitch::mouseDown (const juce::MouseEvent& e)
{
    attachment.setValueAsCompleteGesture (e.x > getWidth() / 2 ? 1.0f : 0.0f);
}

void ModeSwitch::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    const float half = r.getWidth() / 2.0f;

    g.setColour (juce::Colour (0xff0d0518));
    g.fillRoundedRectangle (r, r.getHeight() / 2.0f);
    g.setColour (EclipseColours::track);
    g.drawRoundedRectangle (r, r.getHeight() / 2.0f, 1.5f);

    auto pill = juce::Rectangle<float> (r.getX() + (hard ? half : 0.0f), r.getY(), half, r.getHeight()).reduced (3.0f);
    g.setGradientFill (juce::ColourGradient (purple, pill.getX(), pill.getY(),
                                             violet, pill.getRight(), pill.getBottom(), false));
    g.fillRoundedRectangle (pill, pill.getHeight() / 2.0f);

    g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    g.setColour (hard ? text.withAlpha (0.55f) : juce::Colours::white);
    g.drawText ("SOFT", juce::Rectangle<float> (r.getX(), r.getY(), half, r.getHeight()), juce::Justification::centred);
    g.setColour (hard ? juce::Colours::white : text.withAlpha (0.55f));
    g.drawText ("HARD", juce::Rectangle<float> (r.getX() + half, r.getY(), half, r.getHeight()), juce::Justification::centred);
}

//==============================================================================
EclipseAudioProcessorEditor::EclipseAudioProcessorEditor (EclipseAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), curve (p.apvts), mode (p.apvts),
      clipAttach (p.apvts, "drive", clipKnob),
      outAttach  (p.apvts, "output", outKnob)
{
    setLookAndFeel (&laf);

    auto setupKnob = [this] (juce::Slider& s, juce::Label& l, const juce::String& name, const juce::String& suffix)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
        s.setTextValueSuffix (suffix);
        s.setNumDecimalPlacesToDisplay (0);
        addAndMakeVisible (s);

        l.setText (name, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, text);
        l.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        addAndMakeVisible (l);
    };

    setupKnob (clipKnob, clipLabel, "CLIP", "%");
    setupKnob (outKnob,  outLabel,  "OUTPUT", " dB");
    outKnob.setDoubleClickReturnValue (true, 0.0);
    clipKnob.setDoubleClickReturnValue (true, 50.0);

    addAndMakeVisible (curve);
    addAndMakeVisible (mode);

    setSize (420, 520);
}

EclipseAudioProcessorEditor::~EclipseAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void EclipseAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (bgTop, 0.0f, 0.0f, bgBottom, 0.0f, (float) getHeight(), false));
    g.fillAll();

    // eclipse logo: dark disc with a glowing purple corona
    const float cx = 46.0f, cy = 44.0f;
    for (int i = 6; i > 0; --i)
    {
        g.setColour (purple.withAlpha (0.06f * (float) (7 - i)));
        g.drawEllipse (cx - 14.0f - (float) i, cy - 14.0f - (float) i, 28.0f + 2.0f * (float) i, 28.0f + 2.0f * (float) i, 2.0f);
    }
    g.setColour (glow);
    g.drawEllipse (cx - 14.0f, cy - 14.0f, 28.0f, 28.0f, 2.5f);
    g.setColour (bgBottom);
    g.fillEllipse (cx - 13.0f, cy - 13.0f, 26.0f, 26.0f);

    g.setColour (text);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText ("ECLIPSE", 78, 22, 220, 40, juce::Justification::centredLeft);
}

void EclipseAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (24);
    r.removeFromTop (50); // header

    curve.setBounds (r.removeFromTop (120));
    r.removeFromTop (14);

    mode.setBounds (r.removeFromTop (40).withSizeKeepingCentre (200, 40));
    r.removeFromTop (6);

    auto knobs = r;
    auto left  = knobs.removeFromLeft (int (knobs.getWidth() * 0.62f));

    clipLabel.setBounds (left.removeFromBottom (22));
    clipKnob.setBounds (left);

    auto right = knobs.withTrimmedTop (50);
    outLabel.setBounds (right.removeFromBottom (22));
    outKnob.setBounds (right);
}
