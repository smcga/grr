#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
#include <iostream>
#include <memory>
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
void set(juce::AudioProcessor& p, const juce::String& id,float val){for(auto* parameter:p.getParameters())if(auto* q=dynamic_cast<juce::AudioProcessorParameterWithID*>(parameter))if(q->paramID==id){q->setValueNotifyingHost(val);return;}throw std::runtime_error("missing parameter");}
int main(){
 auto a=std::unique_ptr<juce::AudioProcessor>(createPluginFilter()),b=std::unique_ptr<juce::AudioProcessor>(createPluginFilter());set(*a,"mode",0);set(*b,"mode",.5f);set(*a,"mix",1);set(*b,"mix",1);a->prepareToPlay(48000,128);b->prepareToPlay(48000,128);double before=0,after=0,peak=0;juce::MidiBuffer midi;
 for(int t=0;t<48000*3;t+=128){int count=std::min(128,48000*3-t);juce::AudioBuffer<float> x(2,count),y(2,count);for(int i=0;i<count;++i){float v=(t+i<2400)?float(.5*std::sin(2*juce::MathConstants<double>::pi*440*(t+i)/48000.)):0; x.setSample(0,i,v);x.setSample(1,i,v);y.setSample(0,i,v);y.setSample(1,i,v);}a->processBlock(x,midi);b->processBlock(y,midi);for(int i=0;i<count;++i){double v=y.getSample(0,i),d=std::abs(v-x.getSample(0,i));if(!std::isfinite(v))return 1;peak=std::max(peak,std::abs(v));if(t+i<38400)before=std::max(before,d);if(t+i>48000)after=std::max(after,d);}}
 std::cout<<"pre-motion difference="<<before<<" post-motion difference="<<after<<" peak="<<peak<<"\n";if(before>1e-6||after<1e-5||peak>1)return 2;
 juce::MemoryBlock data;b->getStateInformation(data);auto restored=std::unique_ptr<juce::AudioProcessor>(createPluginFilter());restored->setStateInformation(data.getData(),int(data.getSize()));for(auto* q:restored->getParameters())if(auto* id=dynamic_cast<juce::AudioProcessorParameterWithID*>(q))if(id->paramID=="mix"&&q->getValue()<.99f)return 3;
 std::cout<<"processor smoke checks passed\n";
}
