/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope{
    Slope_6,
    Slope_12,
    Slope_18,
    Slope_24
};

struct ChainSettings{
    float peakFreq {0}, peakGainInDecibels {0}, peakQ {1.0f};
    float lowCutFreq {0}, highCutFreq {0};
    
    Slope lowCutSlope {Slope::Slope_6}, highCutSlope {Slope::Slope_6};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class EQVSTAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EQVSTAudioProcessor();
    ~EQVSTAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};

private:
    
    using Filter = juce::dsp::IIR::Filter<float>;
    
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter>;
    
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    
    MonoChain leftChain, rightChain;
    
    enum ChainPositions{
        LowCut,
        Peak,
        HighCut
    };
    
    void updatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoeffs(Coefficients& old, const Coefficients& updated);
    
    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftLowCut, const CoefficientType& cutCoeffs, Slope& lowCutSlope){
        leftLowCut.template setBypassed<0>(true);
        leftLowCut.template setBypassed<1>(true);
        
        switch (lowCutSlope) { //make this prettier at some point <3
            case Slope_6:
                *leftLowCut.template get<0>().coefficients = *cutCoeffs[0];
                leftLowCut.template setBypassed<0>(false);
                break;
            case Slope_12:
                *leftLowCut.template get<0>().coefficients = *cutCoeffs[0];
                leftLowCut.template setBypassed<0>(false);
                break;
            case Slope_18:
                *leftLowCut.template get<0>().coefficients = *cutCoeffs[0];
                leftLowCut.template setBypassed<0>(false);
                *leftLowCut.template get<1>().coefficients = *cutCoeffs[1];
                leftLowCut.template setBypassed<1>(false);
                break;
            case Slope_24:
                *leftLowCut.template get<0>().coefficients = *cutCoeffs[0];
                leftLowCut.template setBypassed<0>(false);
                *leftLowCut.template get<1>().coefficients = *cutCoeffs[1];
                leftLowCut.template setBypassed<1>(false);
                break;
        }
    }
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQVSTAudioProcessor)
};
