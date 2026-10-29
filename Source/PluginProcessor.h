#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Dsp.h"
class GhostProcessor : public juce::AudioProcessor {
public:
 GhostProcessor();
 void prepareToPlay(double, int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "GHOST ROOM REWRITES";}
 bool acceptsMidi() const override {return false;} bool producesMidi() const override {return false;} bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 20.0;}
 int getNumPrograms() override {return 1;} int getCurrentProgram() override {return 0;} void setCurrentProgram(int) override {} const juce::String getProgramName(int) override {return {}; } void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState state;
 static juce::AudioProcessorValueTreeState::ParameterLayout parameters();
private:
 grr::Engine engine;
 std::array<std::atomic<float>*, 27> params{};
 std::atomic<float>* tone4 = nullptr;
 float value(size_t index) const { return params[index]->load(std::memory_order_relaxed); }
};
