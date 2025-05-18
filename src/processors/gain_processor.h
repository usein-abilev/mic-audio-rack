#include <juce_dsp/juce_dsp.h>

#include "base_processor.h"

class GainProcessor : public ProcessorBase {
   public:
    GainProcessor() = default;

    void setGainDecibels(float decibels) {
        gain.setGainDecibels(decibels);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
        gain.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        gain.process(context);
    }

    void reset() override {
        gain.reset();
    }

    const juce::String getName() const override { return "Gain"; }

   private:
    juce::dsp::Gain<float> gain;
};
