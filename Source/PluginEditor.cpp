#include "PluginEditor.h"
GhostEditor::GhostEditor(GhostProcessor& p):AudioProcessorEditor(&p),processor(p){
 addSwitch("ghost","GHOST"); addSwitch("room","ROOM"); addSwitch("rewrite","REWRITE"); addSwitch("serial","SERIAL (off = parallel)");
 addKnob("mix","MASTER MIX"); addKnob("gmix","GHOST LEVEL"); addKnob("gtime","TIME (s)"); addKnob("gfeedback","FEEDBACK"); addKnob("mutation","MUTATION");
 addKnob("rmix","ROOM LEVEL"); addKnob("size","DECAY"); addKnob("motion","MOTION"); addKnob("material","MATERIAL");
 addKnob("wmix","REWRITE LEVEL"); addKnob("threshold","THRESHOLD"); addKnob("hold","SPACING (s)");
 for(int i=1;i<=4;++i){auto n=juce::String(i); addKnob(("level"+n).toRawUTF8(),("TAIL "+n+" LEVEL").toRawUTF8()); addKnob(("decay"+n).toRawUTF8(),("TAIL "+n+" DECAY").toRawUTF8()); addKnob(("tone"+n).toRawUTF8(),("TAIL "+n+" TONE").toRawUTF8());}
 setSize(1000,650);
}
void GhostEditor::addKnob(const char* id,const char* name){auto k=std::make_unique<Knob>(); k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,64,18); k->label.setText(name,juce::dontSendNotification); k->label.setJustificationType(juce::Justification::centred); k->label.setFont(juce::FontOptions(11.0f)); addAndMakeVisible(k->slider); addAndMakeVisible(k->label); k->attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,k->slider); knobs.push_back(std::move(k));}
void GhostEditor::addSwitch(const char* id,const char* name){auto s=std::make_unique<Switch>(); s->button.setButtonText(name); addAndMakeVisible(s->button); s->attachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,id,s->button); switches.push_back(std::move(s));}
void GhostEditor::paint(juce::Graphics& g){g.fillAll(juce::Colour(0xff10151e)); g.setColour(juce::Colour(0xffd8e5ef)); g.setFont(juce::FontOptions(24.f,juce::Font::bold)); g.drawText("GHOST ROOM REWRITES",20,10,600,35,juce::Justification::left); g.setFont(juce::FontOptions(13.f)); g.setColour(juce::Colour(0xff8ea8b9)); g.drawText("Four rotating event memories  |  adjust a tail while it is sounding",20,47,700,22,juce::Justification::left); g.setColour(juce::Colour(0xff2c3945)); g.drawHorizontalLine(135,20,980); g.drawHorizontalLine(335,20,980); g.setColour(juce::Colour(0xff92c7ce)); g.drawText("GHOST",20,145,200,20,juce::Justification::left); g.drawText("ROOM",360,145,200,20,juce::Justification::left); g.drawText("REWRITE  /  CAPTURE",690,145,290,20,juce::Justification::left); g.drawText("REWRITE  /  TAIL MEMORIES 1-4",20,345,500,20,juce::Justification::left);}
void GhostEditor::resized(){switches[0]->button.setBounds(20,85,130,30); switches[1]->button.setBounds(160,85,120,30); switches[2]->button.setBounds(290,85,150,30); switches[3]->button.setBounds(460,85,220,30);
 auto place=[&](int i,int x,int y,int width=100){knobs[size_t(i)]->label.setBounds(x,y,width,20); knobs[size_t(i)]->slider.setBounds(x,y+20,width,110);};
 place(0,870,5,100); for(int i=0;i<4;++i)place(i+1,20+i*82,175,80); for(int i=0;i<4;++i)place(i+5,360+i*80,175,78); for(int i=0;i<3;++i)place(i+9,690+i*90,175,88);
 for(int i=0;i<12;++i)place(i+12,20+i*80,385,78);
}
