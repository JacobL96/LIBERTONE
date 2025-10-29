#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class LIBERTONEAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    LIBERTONEAudioProcessorEditor(LIBERTONEAudioProcessor&);
    ~LIBERTONEAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent& e) override
    {
        repaint();
    }

private:
    LIBERTONEAudioProcessor& audioProcessor;

    GlowBarLook glowLook;

    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    juce::Slider outputSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;

    juce::ComboBox modeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LIBERTONEAudioProcessorEditor)
};
