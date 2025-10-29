#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================

LIBERTONEAudioProcessor::LIBERTONEAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "PARAMETERS", createParameters())
{
}

LIBERTONEAudioProcessor::~LIBERTONEAudioProcessor() {}

//==============================================================================

bool LIBERTONEAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* LIBERTONEAudioProcessor::createEditor()
{
    return new LIBERTONEAudioProcessorEditor(*this);
}


juce::AudioProcessorValueTreeState::ParameterLayout LIBERTONEAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 10.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("output", "Output", -24.0f, 6.0f, 0.0f)); // dB trim
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "mode", "Mode",
        juce::StringArray{ "Warm Analog", "Bit Fizz", "Tube Comp", "Hard Clip" }, 0));

    return { params.begin(), params.end() };
}

//==============================================================================
// waveshaper
static inline float shapeSample(float x, int mode)
{
    switch (mode)
    {
    case 0: return std::tanh(x * 2.8f);                                   // warm
    case 1: return std::round(x * 8.0f) / 8.0f;                            // bit crush
    case 2: return (2.0f / juce::MathConstants<float>::pi) * std::atan(x * 4.0f); // tube
    case 3: return juce::jlimit(-0.8f, 0.8f, x * 3.0f);                    // hard clip
    default: return x;
    }
}

//==============================================================================

void LIBERTONEAudioProcessor::prepareToPlay(double, int) {}
void LIBERTONEAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LIBERTONEAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
}
#endif

void LIBERTONEAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalCh = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    const float drive = apvts.getRawParameterValue("gain")->load();
    const float mix = apvts.getRawParameterValue("mix")->load();
    const float outputDb = apvts.getRawParameterValue("output")->load();
    const float outputGain = juce::Decibels::decibelsToGain(outputDb);
    const int mode = (int)apvts.getRawParameterValue("mode")->load();

    for (int ch = 0; ch < totalCh; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float dry = data[i];
            float wet = shapeSample(dry * drive, mode);
            data[i] = juce::jlimit(-1.0f, 1.0f, (dry * (1.0f - mix) + wet * mix) * outputGain);
        }
    }
}

//==============================================================================

const juce::String LIBERTONEAudioProcessor::getName() const { return JucePlugin_Name; }
bool LIBERTONEAudioProcessor::acceptsMidi() const { return false; }
bool LIBERTONEAudioProcessor::producesMidi() const { return false; }
bool LIBERTONEAudioProcessor::isMidiEffect() const { return false; }
double LIBERTONEAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int LIBERTONEAudioProcessor::getNumPrograms() { return 1; }
int LIBERTONEAudioProcessor::getCurrentProgram() { return 0; }
void LIBERTONEAudioProcessor::setCurrentProgram(int) {}
const juce::String LIBERTONEAudioProcessor::getProgramName(int) { return {}; }
void LIBERTONEAudioProcessor::changeProgramName(int, const juce::String&) {}

void LIBERTONEAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}
void LIBERTONEAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml) apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new LIBERTONEAudioProcessor(); }
