#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LIBERTONEAudioProcessorEditor::LIBERTONEAudioProcessorEditor(LIBERTONEAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // ===== Drive (Gain) =====
    gainSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    gainSlider.setRange(0.0f, 24.0f, 0.1f); // dB range for drive
    gainSlider.setSkewFactorFromMidPoint(6.0f);
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "gain", gainSlider);

    // ===== Mix =====
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    mixSlider.setRange(0.0f, 100.0f, 1.0f);
    addAndMakeVisible(mixSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "mix", mixSlider);

    // ===== Output Trim =====
    outputSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    outputSlider.setRange(-24.0f, 6.0f, 0.1f); // trim output to prevent clipping
    addAndMakeVisible(outputSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "output", outputSlider);

    // ===== Mode Menu =====
    modeBox.addItemList({ "Warm Analog", "Bit Fizz", "Tube Comp", "Hard Clip" }, 1);
    addAndMakeVisible(modeBox);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "mode", modeBox);

    // Editor window
    setSize(600, 400);
}

LIBERTONEAudioProcessorEditor::~LIBERTONEAudioProcessorEditor() {}

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

    // Title
    g.setColour(juce::Colours::white.withMultipliedAlpha(0.95f));
    g.setFont(juce::Font("Orbitron", 22.0f, juce::Font::bold));
    g.drawText("LIBERTONE Saturator", 24, 16, getWidth() - 48, 28, juce::Justification::centredLeft);

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
