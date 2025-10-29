#include "PluginProcessor.h"
#include "PluginEditor.h"

// ================= CUSTOM LOOK AND FEEL ===================
class GlowBarLook : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
        float sliderPos,
        float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style,
        juce::Slider& slider) override
    {
        // Background bar
        juce::Rectangle<float> bg(x + w * 0.3f, y, w * 0.4f, h);
        g.setColour(juce::Colours::darkgrey.withAlpha(0.35f));
        g.fillRoundedRectangle(bg, 6.0f);

        // Determine glow color from value
        float range = slider.getMaximum() - slider.getMinimum();
        float t = (range > 0.0f) ? (slider.getValue() - slider.getMinimum()) / range : 0.0f;

        juce::Colour heat = juce::Colour::fromHSV(0.85f - t * 0.35f, 0.9f, 0.95f, 1.0f);

        // Fill bar
        juce::Rectangle<float> fill(bg);
        fill.removeFromBottom(bg.getHeight() * (1.0f - t));

        juce::ColourGradient grad(heat.brighter(), fill.getX(), fill.getY(),
            heat.darker(), fill.getX(), fill.getBottom(), false);

        g.setGradientFill(grad);
        g.fillRoundedRectangle(fill, 6.0f);

        // Neon aura
        g.setColour(heat.withAlpha(0.25f));
        g.fillEllipse(bg.getX() - 10, fill.getY() - 6, bg.getWidth() + 20, fill.getHeight() + 12);

        // Glow outline
        g.setColour(heat.withAlpha(0.65f));
        g.drawRoundedRectangle(bg, 6.0f, 2.0f);

        // Edge shadow for 3D effect
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.drawLine(bg.getRight(), bg.getY(), bg.getRight(), bg.getBottom(), 2.0f);

    }
};



//==============================================================================
LIBERTONEAudioProcessorEditor::LIBERTONEAudioProcessorEditor(LIBERTONEAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // ===== Drive (Gain) =====
    gainSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    gainSlider.setRange(0.0f, 24.0f, 0.1f); // dB range for drive
    gainSlider.setSkewFactorFromMidPoint(6.0f);
    gainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::orchid);
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "gain", gainSlider);

    // ===== Mix =====
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    mixSlider.setRange(0.0f, 100.0f, 1.0f);
    mixSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::orchid);
    addAndMakeVisible(mixSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "mix", mixSlider);

    // ===== Output Trim =====
    outputSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    outputSlider.setRange(-24.0f, 6.0f, 0.1f); // trim output to prevent clipping
    outputSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::orchid);
    addAndMakeVisible(outputSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "output", outputSlider);

    // ===== Mode Menu =====
    modeBox.addItemList({ "Warm Analog", "Bit Fizz", "Tube Comp", "Hard Clip" }, 1);
    addAndMakeVisible(modeBox);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "mode", modeBox);

    setLookAndFeel(&glowLook);

    // Editor window
    setSize(600, 400);
}

LIBERTONEAudioProcessorEditor::~LIBERTONEAudioProcessorEditor() { setLookAndFeel(nullptr); }

//==============================================================================
void LIBERTONEAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background gradient
    juce::Colour top = juce::Colour::fromRGB(20, 10, 30);
    juce::Colour bottom = juce::Colour::fromRGB(60, 10, 80);
    juce::ColourGradient gradient(top, 0, 0, bottom, 0, (float)getHeight(), false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    // Main panel
    juce::Rectangle<int> panel = getLocalBounds().reduced(16);
    g.setColour(juce::Colour::fromRGBA(0, 0, 0, 120));
    g.fillRoundedRectangle(panel.toFloat(), 12.0f);

    auto mouse = getMouseXYRelative();
    if (getLocalBounds().contains(mouse))
    {
        g.setColour(juce::Colour::fromRGBA(255, 0, 180, 25));
        g.fillRoundedRectangle(panel.toFloat(), 12.0f);
    }

    // Title
    g.setColour(juce::Colour::fromRGB(255, 180, 255));
    g.setFont(juce::Font("Orbitron", 24.0f, juce::Font::bold));
    g.drawText("LIBERTONE SATURATOR", 24, 16, getWidth() - 48, 28, juce::Justification::centredLeft);

    // Sub text glow
    g.setColour(juce::Colour::fromRGBA(255, 100, 255, 180));
    g.setFont(14.0f);

    // Labels
    g.setFont(14.0f);
    g.setColour(juce::Colours::white.withMultipliedAlpha(0.85f));
    g.drawFittedText("Drive", gainSlider.getX(), gainSlider.getY() - 20, gainSlider.getWidth(), 18, juce::Justification::centred, 1);
    g.drawFittedText("Mix", mixSlider.getX(), mixSlider.getY() - 20, mixSlider.getWidth(), 18, juce::Justification::centred, 1);
    g.drawFittedText("Output", outputSlider.getX(), outputSlider.getY() - 20, outputSlider.getWidth(), 18, juce::Justification::centred, 1);
    g.drawFittedText("Mode", modeBox.getX(), modeBox.getY() - 22, modeBox.getWidth(), 18, juce::Justification::centred, 1);
}

void LIBERTONEAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(30);
    int barW = 60;
    int barH = 220;
    int knobSize = 110;

    // Layout
    int topY = 60;

    gainSlider.setBounds(100, topY, barW, barH);
    mixSlider.setBounds(240, topY + 40, knobSize, knobSize);
    outputSlider.setBounds(410, topY, barW, barH);

    modeBox.setBounds(240, 310, 120, 24);
}
