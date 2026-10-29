#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

constexpr double pi=3.14159265358979323846, speed=343.0;
struct Motion {
 double length, delta, start, duration;
 double pos(double t) const { double u=std::clamp((t-start)/duration,0.0,1.0); return length+delta*u*u*(3-2*u); }
 double vel(double t) const { double u=(t-start)/duration; return (u>0&&u<1)?delta*6*u*(1-u)/duration:0; }
};
// Left-going/right-going pressure waves in a 1D duct. At x=0, the source and left
// wall scatter. At x=L(t), incident energy is reflected using encounter time.
// Every wall return is scheduled by solving t = tau + L(tau)/c, not by
// changing a delay applied to an already completed room impulse response.
struct Duct {
 int fs; double reflect, wallGain; Motion m; std::vector<double> outgoing, wall;
 Duct(int sr,int samples,Motion motion,double r,double wg=1):fs(sr),reflect(r),wallGain(wg),m(motion),outgoing(samples,0),wall(samples,0){}
 static double sample(const std::vector<double>& x,double index,int maxIndex) {
  if(index<0 || index>maxIndex) return 0;
  int i=int(index); double f=index-i; if(i>=maxIndex) return x[size_t(i)];
  return x[size_t(i)]*(1-f)+x[size_t(i+1)]*f;
 }
 double tick(int n,double input){
  double t=double(n)/fs;
  // Outgoing wave reaching the moving wall, evaluated at its physical encounter.
  double incident=sample(outgoing,(t-m.pos(t)/speed)*fs,n-1);
  double beta=m.vel(t)/speed;
  // Pressure reflection from a moving rigid wall in this 1D travelling-wave model.
  wall[size_t(n)]=wallGain*reflect*incident*(1-beta)/(1+beta);
  // Arrival at x=0: find actual reflection time on the moving wall worldline.
  double tau=t-m.pos(t)/speed;
  for(int j=0;j<5;++j) tau-=(tau+m.pos(tau)/speed-t)/(1+m.vel(tau)/speed);
  double incoming=sample(wall,tau*fs,n);
  outgoing[size_t(n)]=input+reflect*incoming;
  return incoming;
 }
};
using Audio=std::vector<std::array<double,2>>;
static uint32_t rng=0x38a9e3u;
double random01(){rng^=rng<<13; rng^=rng>>17; rng^=rng<<5; return double(rng)/4294967296.0;}
std::vector<double> source(int fs,int n){
 std::vector<double> x(size_t(n),0); double z=0, last=0; rng=0x38a9e3u;
 for(int i=0;i<int(.18*fs);++i){double t=double(i)/fs; double noise=random01()*2-1; z+=.35*(noise-z); double hp=z-last; last=z; double body=std::sin(2*pi*(180*t-250*t*t))*std::exp(-t*30); x[size_t(i)]=.7*hp*std::exp(-t*23)+.35*body; }
 return x;
}
Audio render(int fs,const std::vector<double>& x,double delta,double start,double duration,bool finalStatic=false,bool active=false){
 int n=int(x.size()); Audio y(size_t(n),{0,0});
 // A small bundle of 1-D propagation paths is a parallel-bank artistic model,
 // not a coupled 3-D room. Each path has distinct length and pickup weight.
 const double lengths[8]={4.3,5.7,6.8,8.1,9.7,11.3,13.1,15.2};
 for(int k=0;k<8;++k){double d=delta*(.82+.05*k); Motion m{lengths[k]+(finalStatic?d:0),finalStatic?0:d,start,duration}; Duct duct(fs,n,m,active?.95:.992,active?1.1:1.0);
  double gl=(k%2==0?.28:.16),gr=(k%2==0?.16:.28); double sign=(k%3==0?-1:1);
  for(int i=0;i<n;++i){double w=duct.tick(i,x[size_t(i)]); y[size_t(i)][0]+=w*gl; y[size_t(i)][1]+=w*gr*sign;}
 } return y;
}
void u16(std::ofstream& o,uint16_t v){for(int i=0;i<2;++i)o.put(char((v>>(8*i))&255));}
void u32(std::ofstream& o,uint32_t v){for(int i=0;i<4;++i)o.put(char((v>>(8*i))&255));}
void wav(const std::filesystem::path& p,const Audio& a,int fs){std::ofstream o(p,std::ios::binary); if(!o)throw std::runtime_error("Cannot write WAV"); uint32_t bytes=uint32_t(a.size()*4); o.write("RIFF",4);u32(o,36+bytes);o.write("WAVEfmt ",8);u32(o,16);u16(o,1);u16(o,2);u32(o,uint32_t(fs));u32(o,uint32_t(fs*4));u16(o,4);u16(o,16);o.write("data",4);u32(o,bytes); for(auto x:a)for(double v:x){v=std::clamp(v,-1.0,1.0); int16_t s=int16_t(std::lround(v*32767));u16(o,uint16_t(s));}}
double rms(const Audio& a,int begin){double sum=0;for(size_t i=size_t(begin);i<a.size();++i)sum+=a[i][0]*a[i][0]+a[i][1]*a[i][1];return std::sqrt(sum/(2*(a.size()-size_t(begin))));}
double peak(const Audio& a){double p=0;for(auto v:a)for(double c:v)p=std::max(p,std::abs(c));return p;}
int tests(){
 const int fs=48000,n=48000; // Constant velocity begins long before first reflection.
 // Linear trajectory embedded in the cubic by testing a local known-time reflection
 // against an independent root of the pulse worldline.
 Motion m{30,12,0,.6}; Duct d(fs,n,m,1); std::vector<double> out(n);
 for(int i=0;i<n;++i)out[size_t(i)]=d.tick(i,i==0?1:0);
 double lo=0,hi=.5;for(int i=0;i<60;++i){double t=(lo+hi)*.5;if(speed*t<m.pos(t))lo=t;else hi=t;}
 double hit=(lo+hi)*.5, arrival=hit+m.pos(hit)/speed;
 int found=int(std::max_element(out.begin()+int(.08*fs),out.begin()+int(.3*fs))-out.begin());
 double error=std::abs(double(found)/fs-arrival); std::cout<<"reflection predicted="<<arrival<<"s measured="<<double(found)/fs<<"s error="<<error*1000<<"ms\n";
 if(error>2.0/fs)return 1;
 // Fixed wall, cosine crossing at receiver: no reflection until 2L/c.
 Motion fixed{30,0,0,1}; Duct f(fs,n,fixed,1);int first=-1;for(int i=0;i<n;++i){double y=f.tick(i,i==0?1:0); if(first<0&&std::abs(y)>1e-6)first=i;}
 if(std::abs(double(first)/fs-60/speed)>2.0/fs)return 2;
 // At a receding wall, phase mapping implies output frequency ratio (c-v)/(c+v).
 // Verify from zero crossings, with a steady sinusoid and local mean wall velocity.
 Motion linear{15,24,0,1}; Duct tone(fs,n,linear,.9); std::vector<double> y(n);for(int i=0;i<n;++i)y[size_t(i)]=tone.tick(i,std::sin(2*pi*700*i/fs));
 std::vector<int> crossings;for(int i=int(.22*fs);i<int(.25*fs);++i)if(y[size_t(i-1)]<=0 && y[size_t(i)]>0)crossings.push_back(i);
 if(crossings.size()<5)return 3;double freq=double(fs)*(crossings.size()-1)/(crossings.back()-crossings.front());
 double tmid=double(crossings.front()+crossings.back())/(2*fs),tau=tmid-linear.pos(tmid)/speed;for(int j=0;j<8;++j)tau-=(tau+linear.pos(tau)/speed-tmid)/(1+linear.vel(tau)/speed);
 double v=linear.vel(tau), expected=700*(speed-v)/(speed+v);std::cout<<"doppler measured="<<freq<<"Hz predicted="<<expected<<"Hz wall velocity="<<v<<"m/s\n";
 if(std::abs(freq-expected)>15)return 4;
 // Moving geometry cannot affect samples before the movement starts.
 auto pulse=source(fs,fs*3); auto a=render(fs,pulse,0,.8,1); auto b=render(fs,pulse,15,.8,1); double before=0,after=0,maximum=0;
 for(int i=0;i<fs*3;++i){for(int ch=0;ch<2;++ch){double diff=std::abs(a[size_t(i)][ch]-b[size_t(i)][ch]); if(i<int(.8*fs))before=std::max(before,diff); else after=std::max(after,diff); maximum=std::max(maximum,std::abs(b[size_t(i)][ch])); if(!std::isfinite(b[size_t(i)][ch]))return 5;}}
 auto endpoint=render(fs,pulse,15,.8,1,true); double crossDiff2=0; int count=0;
 for(int i=int(.8*fs);i<int(3*fs);++i){double u=std::clamp((double(i)/fs-.8),0.0,1.0);u=u*u*(3-2*u);for(int ch=0;ch<2;++ch){double conventional=a[size_t(i)][ch]*(1-u)+endpoint[size_t(i)][ch]*u; double e=b[size_t(i)][ch]-conventional;crossDiff2+=e*e;++count;}}
 std::cout<<"pre-motion difference="<<before<<", post-motion difference="<<after<<", peak="<<maximum<<", moving-minus-crossfade RMS="<<std::sqrt(crossDiff2/count)<<"\n";
 if(before>1e-12||after<1e-6||maximum>2)return 6;
 // Repeat the first-arrival calculation at a second sample rate.
 int fs2=96000; Duct d2(fs2,fs2,m,1); double maxv=0;int at=0; for(int i=0;i<int(.3*fs2);++i){double q=std::abs(d2.tick(i,i==0?1:0)); if(i>int(.08*fs2)&&q>maxv){maxv=q;at=i;}}
 double e2=std::abs(double(at)/fs2-arrival);std::cout<<"96k arrival error="<<e2*1000<<"ms\n"; if(e2>2.0/fs2)return 7;
 // The intentionally active wall returns more pressure than arrived at it.
 Motion activeMotion{3.43,0,0,1}; Duct activeDuct(fs,fs,activeMotion,.95,1.1); for(int i=0;i<1000;++i)activeDuct.tick(i,i==0?1:0);
 double activePeak=*std::max_element(activeDuct.wall.begin(),activeDuct.wall.begin()+1000); std::cout<<"active wall reflection="<<activePeak<<" (passive would be <= 1)\n";if(activePeak<1.04)return 8;
 std::cout<<"physical checks passed\n";return 0;
}
int main(int argc,char** argv){try{
 if(argc>1&&std::string(argv[1])=="--test")return tests();
 std::filesystem::path dir=argc>1?argv[1]:"renders"; std::filesystem::create_directories(dir);int fs=48000,n=fs*10;auto begin=std::chrono::steady_clock::now();auto x=source(fs,n);
 auto a=render(fs,x,0,.8,1.0), b=render(fs,x,15,.8,1.0,true), moving=render(fs,x,15,.8,1.0), extreme=render(fs,x,-3.5,.8,.35,false,true);
 // Crossfade two static systems that were both excited by the same source.
 Audio base=a;for(int i=0;i<n;++i){double u=std::clamp((double(i)/fs-.8)/1.0,0.0,1.0);u=u*u*(3-2*u);for(int c=0;c<2;++c)base[size_t(i)][c]=a[size_t(i)][c]*(1-u)+b[size_t(i)][c]*u;}
 // Whole-tail RMS matching, one constant scalar per file; then a shared peak cap.
 std::array<Audio*,4> all{&a,&moving,&base,&extreme};double target=rms(a,int(.8*fs));std::array<double,4> gains{};double maxp=0;
 for(size_t j=0;j<all.size();++j){gains[j]=target/std::max(1e-20,rms(*all[j],int(.8*fs)));maxp=std::max(maxp,peak(*all[j])*gains[j]);}
 double master=.8/std::max(maxp,1e-20);for(size_t j=0;j<all.size();++j){gains[j]*=master;for(auto& sample:*all[j])for(double& c:sample)c*=gains[j];}
 Audio src(size_t(n),{0,0});for(int i=0;i<n;++i)src[size_t(i)]={x[size_t(i)]*.5,x[size_t(i)]*.5};
 wav(dir/"00-source.wav",src,fs); wav(dir/"01-static.wav",a,fs);wav(dir/"02-moving-wall.wav",moving,fs);wav(dir/"03-static-ir-crossfade.wav",base,fs);wav(dir/"04-impossible-collapse.wav",extreme,fs);
 auto secs=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();std::cout<<std::setprecision(8)<<"source ends 0.18s; movement begins 0.8s\n"<<"tail RMS matched from 0.8s; gains static/moving/crossfade/collapse: ";for(auto g:gains)std::cout<<g<<" ";std::cout<<"\nrender seconds="<<secs<<" for 10s audio, 4 room simulations; max peak="<<.8<<"\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
