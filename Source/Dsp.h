#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace grr {
inline float clamp(float x, float lo, float hi) { return std::max(lo, std::min(hi,x)); }
struct Stereo { float l=0, r=0; };
inline Stereo operator+(Stereo a, Stereo b) { return {a.l+b.l,a.r+b.r}; }
inline Stereo operator*(Stereo a, float b) { return {a.l*b,a.r*b}; }
struct Controls {
 bool ghost=true, room=true, rewrite=true, serial=false;
 float mix=.35f, ghostMix=.7f, roomMix=.7f, rewriteMix=.7f, ghostTime=.35f, ghostFeedback=.55f, ghostTransform=.45f;
 float roomSize=.55f, roomMotion=.35f, roomTone=.55f;
 float threshold=.12f, hold=.25f;
 std::array<float,4> level{{.7f,.7f,.7f,.7f}}, decay{{.65f,.65f,.65f,.65f}}, tone{{.55f,.55f,.55f,.55f}};
};
// Four delay lines with a normalized Hadamard feedback matrix. No heap work in process().
struct Diffuser {
 std::array<std::vector<float>,4> d; std::array<int,4> pos{}, len{}; std::array<float,4> filter{};
 void prepare(double sr) {
  const float t[4]={.0297f,.0371f,.0411f,.0437f};
  for(int i=0;i<4;++i){len[i]=std::max(2,int(sr*t[i])); d[i].assign(len[i],0); pos[i]=0; filter[i]=0;}
 }
 void clear(){for(auto& v:d) std::fill(v.begin(),v.end(),0.0f); pos={}; filter={};}
 Stereo process(Stereo in,float decay,float tone) {
  float a[4]; for(int i=0;i<4;++i){float v=d[i][pos[i]]; filter[i] += (.015f + .44f*clamp(tone,0,1))*(v-filter[i]); a[i]=filter[i];}
  const float m[4]={(a[0]+a[1]+a[2]+a[3])*.5f,(a[0]-a[1]+a[2]-a[3])*.5f,(a[0]+a[1]-a[2]-a[3])*.5f,(a[0]-a[1]-a[2]+a[3])*.5f};
  const float fb=.12f+.86f*clamp(decay,0,1);
  const float x[4]={in.l,in.r,in.l,-in.r};
  for(int i=0;i<4;++i){d[i][pos[i]]=x[i]*.24f+m[i]*fb; if(++pos[i]>=len[i])pos[i]=0;}
  return {(a[0]+a[1]-a[2]-a[3])*.5f,(a[0]-a[1]-a[2]+a[3])*.5f};
 }
};
struct Engine {
 double sr=44100; std::vector<float> dl,dr; int write=0; float lpL=0,lpR=0,env=0; int cooldown=0,current=-1; float phase=0; Diffuser room; std::array<Diffuser,4> voices;
 void prepare(double rate){sr=rate; dl.assign(size_t(rate*2.1)+8,0); dr=dl; write=0; lpL=lpR=env=phase=0; cooldown=0; current=-1; room.prepare(sr); for(auto& v:voices)v.prepare(sr);}
 float read(const std::vector<float>& b, float delay){float p=float(write)-delay; while(p<0)p+=float(b.size()); int i=int(p), j=(i+1)%int(b.size()); float f=p-i; return b[size_t(i)]*(1-f)+b[size_t(j)]*f;}
 Stereo process(Stereo input,const Controls& c){
  phase += float(6.283185307179586*.17/sr); if(phase>6.2831853f) phase-=6.2831853f;
  auto ghostProcess=[&](Stereo x){float samples=clamp(c.ghostTime,.03f,2.f)*float(sr); float mod=std::sin(phase)*c.ghostTransform*float(sr)*.002f;
   Stereo tap{read(dl,samples+mod),read(dr,samples-mod)}; float cutoff=.6f-.57f*clamp(c.ghostTransform,0,1); lpL+=cutoff*(tap.l-lpL); lpR+=cutoff*(tap.r-lpR);
   float drive=1+3*clamp(c.ghostTransform,0,1); float norm=std::tanh(drive); Stereo transformed{std::tanh(lpL*drive)/norm,std::tanh(lpR*drive)/norm};
   dl[size_t(write)]=x.l+transformed.r*clamp(c.ghostFeedback,0,.94f); dr[size_t(write)]=x.r+transformed.l*clamp(c.ghostFeedback,0,.94f); if(++write>=int(dl.size()))write=0; return transformed;};
  auto roomProcess=[&](Stereo x){float motion=std::sin(phase)*clamp(c.roomMotion,0,1)*.12f; return room.process(x,clamp(c.roomSize+motion,0,1),c.roomTone);};
  auto rewriteProcess=[&](Stereo x){float mag=std::max(std::abs(x.l),std::abs(x.r)); float prev=env; float coeff=mag>env?.08f:float(1-std::exp(-1.0/(sr*.08))); env+=coeff*(mag-env); if(cooldown>0)--cooldown;
   if(cooldown==0 && env>c.threshold && prev<=c.threshold){current=(current+1)%4; voices[size_t(current)].clear(); cooldown=int(sr*clamp(c.hold,.05f,2.f));}
   Stereo sum{}; for(int i=0;i<4;++i){Stereo feed=(i==current)?x:Stereo{}; auto v=voices[size_t(i)].process(feed,c.decay[size_t(i)],c.tone[size_t(i)]); sum=sum+v*c.level[size_t(i)];} return sum*.5f;};
  // Bypassed engines are fed silence and continue advancing so old buffers do not freeze.
  Stereo wet{};
  if(c.serial){Stereo chain=input; if(c.ghost)chain=chain+ghostProcess(chain)*c.ghostMix; else ghostProcess({}); if(c.room)chain=chain+roomProcess(chain)*c.roomMix; else roomProcess({}); if(c.rewrite)chain=chain+rewriteProcess(chain)*c.rewriteMix; else rewriteProcess({}); wet={chain.l-input.l,chain.r-input.r};}
  else {int n=int(c.ghost)+int(c.room)+int(c.rewrite); auto g=ghostProcess(c.ghost?input:Stereo{}); auto r=roomProcess(c.room?input:Stereo{}); auto w=rewriteProcess(c.rewrite?input:Stereo{}); if(c.ghost)wet=wet+g*c.ghostMix; if(c.room)wet=wet+r*c.roomMix; if(c.rewrite)wet=wet+w*c.rewriteMix; if(n>0)wet=wet*(1.f/float(n));}
  float mix=clamp(c.mix,0,1); if(!c.ghost&&!c.room&&!c.rewrite)return input; return input*(1-mix)+wet*mix;
 }
};
}
