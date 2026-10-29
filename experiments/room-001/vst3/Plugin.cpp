#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
constexpr double c=343.0;
constexpr double lengths[8]={4.3,5.7,6.8,8.1,9.7,11.3,13.1,15.2};
double smooth(double x){ x=std::clamp(x,0.0,1.0); return x*x*(3-2*x); }
struct Motion {
 double start=1e12;
 double amount(double t) const {double u=t-start;if(u<0)return 0;if(u<1)return smooth(u);if(u<2.2)return 1;if(u<3.2)return 1-smooth(u-2.2);return 0;}
 double slope(double t) const {double u=t-start;if(u>0&&u<1)return 6*u*(1-u);u-=2.2;return u>0&&u<1 ? -6*u*(1-u):0;}
};
struct Duct {
 double fs=48000, length=4, delta=0, reflect=.992; int size=0; long long n=0;
 std::vector<double> outgoing,wall;
 void prepare(double sr, double l,double d){fs=sr;length=l;delta=d;n=0; size=int(sr*.75)+8;outgoing.assign(size_t(size),0);wall.assign(size_t(size),0);}
 double read(const std::vector<double>& buffer,double index,long long latest) const {
  if(index<0 || index>double(latest) || index<double(latest-size+2))return 0;
  auto i=static_cast<long long>(index);double frac=index-double(i);auto at=[&](long long j){return buffer[size_t(j%size)];};
  return i>=latest?at(i):at(i)*(1-frac)+at(i+1)*frac;
 }
 double tick(double x,const Motion& motion){
  double t=double(n)/fs, L=length+delta*motion.amount(t), v=delta*motion.slope(t);
  double incoming=read(outgoing,(t-L/c)*fs,n-1);
  wall[size_t(n%size)]=reflect*incoming*(c-v)/(c+v);
  double tau=t-L/c;
  for(int i=0;i<5;++i){double lt=length+delta*motion.amount(tau), vt=delta*motion.slope(tau);tau-=(tau+lt/c-t)/(1+vt/c);}
  double result=read(wall,tau*fs,n);
  outgoing[size_t(n%size)]=x+reflect*result;
  ++n;return result;
 }
};
struct Bank {
 std::array<Duct,8> ducts;
 void prepare(double sr,int variant){for(int i=0;i<8;++i){double d=15*(.82+.05*i);ducts[size_t(i)].prepare(sr,lengths[i]+(variant==2?d:0),variant==1?d:0);}}
 std::array<double,2> tick(double x,const Motion& m){std::array<double,2> out{};for(int i=0;i<8;++i){double q=ducts[size_t(i)].tick(x,m);double a=i%2==0?.28:.16,b=i%2==0?.16:.28;out[0]+=q*a;out[1]+=q*b*(i%3==0?-1:1);}return out;}
};
}
class RoomProcessor:public juce::AudioProcessor {
public:
 using State=juce::AudioProcessorValueTreeState;
 RoomProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"PARAMETERS",layout()){
  mode=state.getRawParameterValue("mode");mix=state.getRawParameterValue("mix");threshold=state.getRawParameterValue("threshold");output=state.getRawParameterValue("output");
 }
 static State::ParameterLayout layout(){State::ParameterLayout p;
  p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode",1},"Comparison",juce::StringArray{"Static room","Moving walls","Static crossfade"},1));
  p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix",1},"Dry wet",juce::NormalisableRange<float>(0,1),.35f));
  p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"threshold",1},"Trigger threshold",juce::NormalisableRange<float>(.01f,.4f),.08f));
  p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"output",1},"Output trim",juce::NormalisableRange<float>(0,1),.7f));return p;
 }
 void prepareToPlay(double sr,int) override {sampleRate=sr;for(int i=0;i<3;++i)banks[size_t(i)].prepare(sr,i);time=0;env=0;motion.start=1e12;ready=0;}
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout& b) const override {return b.getMainInputChannelSet()==juce::AudioChannelSet::stereo()&&b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&) override {
  juce::ScopedNoDenormals guard; const auto selected=int(mode->load()); const double wet=mix->load(), thresh=threshold->load(), gain=output->load();
  float* l=buffer.getWritePointer(0);float* r=buffer.getWritePointer(1);
  for(int i=0;i<buffer.getNumSamples();++i){double t=double(time)/sampleRate;double inL=l[i],inR=r[i],in=.5*(inL+inR);double prev=env;double magnitude=std::max(std::abs(inL),std::abs(inR));double coeff=magnitude>env?.04:1-std::exp(-1/(sampleRate*.08));env+=coeff*(magnitude-env);
   if(t>=ready&&prev<thresh&&env>=thresh){motion.start=t+.8;ready=motion.start+3.3;}
   auto a=banks[0].tick(in,motion),b=banks[1].tick(in,motion),end=banks[2].tick(in,motion);double blend=motion.amount(t);
   auto sample=[&](int ch){double w=selected==0?a[size_t(ch)]:selected==1?b[size_t(ch)]:a[size_t(ch)]*(1-blend)+end[size_t(ch)]*blend;return std::clamp(w,-2.0,2.0);};
   l[i]=float(std::clamp(((1-wet)*inL+wet*sample(0))*gain,-1.0,1.0));r[i]=float(std::clamp(((1-wet)*inR+wet*sample(1))*gain,-1.0,1.0));++time;
  }
 }
 juce::AudioProcessorEditor* createEditor() override {return new juce::GenericAudioProcessorEditor(*this);} bool hasEditor() const override{return true;}
 const juce::String getName() const override{return "GRR ROOM-001";} bool acceptsMidi()const override{return false;} bool producesMidi()const override{return false;} bool isMidiEffect()const override{return false;}
 double getTailLengthSeconds()const override{return 20;} int getNumPrograms()override{return 1;} int getCurrentProgram()override{return 0;} void setCurrentProgram(int)override{} const juce::String getProgramName(int)override{return {};} void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock& b)override{auto xml=state.copyState().createXml();copyXmlToBinary(*xml,b);}
 void setStateInformation(const void* d,int n)override{auto xml=getXmlFromBinary(d,n);if(xml&&xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
private:
 State state; std::atomic<float>* mode=nullptr;std::atomic<float>* mix=nullptr;std::atomic<float>* threshold=nullptr;std::atomic<float>* output=nullptr;
 std::array<Bank,3> banks;Motion motion;double sampleRate=48000,env=0,ready=0;long long time=0;
};
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new RoomProcessor();}
