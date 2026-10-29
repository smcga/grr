#include "Dsp.h"
#include <cmath>
#include <iostream>
int main(){grr::Engine e; e.prepare(48000); grr::Controls c; c.mix=1; c.ghost=true; c.room=c.rewrite=false; c.ghostTime=.05f; float peak=0; for(int i=0;i<10000;i++){auto y=e.process({i==0?1.f:0.f,0},c); if(!std::isfinite(y.l)||!std::isfinite(y.r))return 1; if(i>2000)peak=std::max(peak,std::max(std::abs(y.l),std::abs(y.r)));} if(peak<.01f)return 2;
 e.prepare(48000); c.ghost=c.room=c.rewrite=false; auto dry=e.process({.3f,-.2f},c); if(std::abs(dry.l-.3f)>1e-6f||std::abs(dry.r+.2f)>1e-6f)return 3;
 e.prepare(48000); c.rewrite=true; c.mix=1; peak=0; for(int i=0;i<24000;i++){float x=i<500?.8f:0; auto y=e.process({x,x},c); peak=std::max(peak,std::abs(y.l)); if(!std::isfinite(y.l))return 4;} if(peak<.001f || e.current!=0)return 5;

 // Allocate two events; changing a silent voice must not change the sounding voice.
 grr::Engine a,b; a.prepare(48000); b.prepare(48000); c.threshold=.12f; c.hold=.05f; c.rewrite=true;
 for(int i=0;i<15000;++i){float x=(i<400 || (i>=8000 && i<8400))?.8f:0.f; auto x1=a.process({x,x},c); auto x2=b.process({x,x},c); if(std::abs(x1.l-x2.l)>1e-6f)return 6;}
 if(a.current!=1)return 7;
 auto changed=c; changed.level[0]=0; float difference=0; for(int i=0;i<5000;++i){auto x1=a.process({},changed); auto x2=b.process({},c); difference+=std::abs(x1.l-x2.l); } if(difference<.00001f)return 8;
 // Exercise all routing combinations for bounded, finite output.
 for(int mask=0;mask<8;++mask)for(int serial=0;serial<2;++serial){e.prepare(48000); c.ghost=mask&1; c.room=mask&2; c.rewrite=mask&4; c.serial=serial!=0; for(int i=0;i<24000;++i){float x=i<1000?.3f:0; auto y=e.process({x,x},c); if(!std::isfinite(y.l)||!std::isfinite(y.r)||std::abs(y.l)>20||std::abs(y.r)>20)return 9;}}
 std::cout << "DSP smoke tests passed\n"; }
