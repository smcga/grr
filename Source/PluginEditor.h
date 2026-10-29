#pragma once
#include "PluginProcessor.h"
class GhostEditor : public juce::AudioProcessorEditor {
public:
 explicit GhostEditor(GhostProcessor&); void paint(juce::Graphics&) override; void resized() override;
private:
 GhostProcessor& processor;
 struct Knob {juce::Slider slider; juce::Label label; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;};
 struct Switch {juce::ToggleButton button; std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;};
 std::vector<std::unique_ptr<Knob>> knobs; std::vector<std::unique_ptr<Switch>> switches;
 void addKnob(const char*,const char*); void addSwitch(const char*,const char*);
};
