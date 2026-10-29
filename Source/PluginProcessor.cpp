#include "PluginProcessor.h"
#include "PluginEditor.h"
using APVTS=juce::AudioProcessorValueTreeState;
GhostProcessor::GhostProcessor(): AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)), state(*this,nullptr,"PARAMETERS",parameters()) {
 const char* ids[]={"ghost","room","rewrite","serial","mix","gmix","gtime","gfeedback","mutation","rmix","size","motion","material","wmix","threshold","hold","level1","decay1","tone1","level2","decay2","tone2","level3","decay3","tone3","level4","decay4"};
 // Tone 4 is cached separately below by expanding the array.
 for(size_t i=0;i<27;++i) params[i]=state.getRawParameterValue(ids[i]);
 tone4=state.getRawParameterValue("tone4");
}
APVTS::ParameterLayout GhostProcessor::parameters(){
 APVTS::ParameterLayout layout;
 auto b=[&](const char* id,const char* name,bool init){layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},name,init));};
 auto f=[&](juce::String id,juce::String name,float lo,float hi,float init){layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(lo,hi),init));};
 b("ghost","GHOST on",true); b("room","ROOM on",true); b("rewrite","REWRITE on",true); b("serial","Serial routing",false);
 f("mix","Master mix",0,1,.35f); f("gmix","Ghost level",0,1,.7f); f("gtime","Delay time (seconds)",.03f,2,.35f); f("gfeedback","Feedback",0,.94f,.55f); f("mutation","Mutation",0,1,.45f);
 f("rmix","Room level",0,1,.7f); f("size","Room decay",0,1,.55f); f("motion","Motion",0,1,.35f); f("material","Material",0,1,.55f);
 f("wmix","Rewrite level",0,1,.7f); f("threshold","Capture threshold",.01f,.8f,.12f); f("hold","Capture spacing (seconds)",.05f,2,.25f);
 for(int i=1;i<=4;++i){auto n=juce::String(i); f("level"+n,"Tail "+n+" level",0,1,.7f); f("decay"+n,"Tail "+n+" decay",0,1,.65f); f("tone"+n,"Tail "+n+" tone",0,1,.55f);}
 return layout;
}
void GhostProcessor::prepareToPlay(double rate,int){engine.prepare(rate);}
bool GhostProcessor::isBusesLayoutSupported(const BusesLayout& l) const {return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo() && l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void GhostProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){
 juce::ScopedNoDenormals noDenormals; grr::Controls c;
 c.ghost=value(0)>.5f; c.room=value(1)>.5f; c.rewrite=value(2)>.5f; c.serial=value(3)>.5f;
 c.mix=value(4); c.ghostMix=value(5); c.ghostTime=value(6); c.ghostFeedback=value(7); c.ghostTransform=value(8); c.roomMix=value(9); c.roomSize=value(10); c.roomMotion=value(11); c.roomTone=value(12); c.rewriteMix=value(13); c.threshold=value(14); c.hold=value(15);
 for(int i=0;i<4;++i){c.level[size_t(i)]=value(size_t(16+i*3)); c.decay[size_t(i)]=value(size_t(17+i*3)); c.tone[size_t(i)]=i==3 ? tone4->load(std::memory_order_relaxed) : value(size_t(18+i*3));}
 auto* l=buffer.getWritePointer(0); auto* r=buffer.getWritePointer(1); for(int i=0;i<buffer.getNumSamples();++i){auto out=engine.process({l[i],r[i]},c); l[i]=out.l; r[i]=out.r;}
}
void GhostProcessor::getStateInformation(juce::MemoryBlock& dest){auto xml=state.copyState().createXml(); copyXmlToBinary(*xml,dest);}
void GhostProcessor::setStateInformation(const void* data,int size){auto xml=getXmlFromBinary(data,size); if(xml && xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessorEditor* GhostProcessor::createEditor(){return new GhostEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new GhostProcessor();}
