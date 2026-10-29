"""REWRITE-003: event-labelled acoustic contributions and imperfect mixture attribution."""
import argparse,json,time
from pathlib import Path
import numpy as np
from scipy import signal, optimize
import soundfile as sf
SR=24000; DURATION=8; N=SR*DURATION; EDIT=2.8; SEED=3911

def chord(notes,start,end,phase=0):
 x=np.zeros(N);n=int((end-start)*SR);t=np.arange(n)/SR
 # Attack + release with equal-frequency overlapping partials create a hard attribution case.
 env=np.minimum(t/.04,1)*np.minimum((n/SR-t)/.12,1)
 y=sum(sum(np.sin(2*np.pi*(440*2**((m-69)/12))*k*t+phase)/k**1.5 for k in range(1,5)) for m in notes)
 x[int(start*SR):int(start*SR)+n]=.055*y*env
 return x

def impulse():
 rng=np.random.default_rng(SEED);t=np.arange(int(5*SR))/SR
 ir=rng.standard_normal(len(t))*np.exp(-t/1.25)*.025
 # Sparse early reflections with a diffuse tail. It is a synthetic linear IR, not a physical room.
 ir[:int(.018*SR)]=0
 for sec,amp in [(0.025,.7),(.041,.5),(.067,.32),(.093,.25)]: ir[int(sec*SR)]+=amp
 return ir

def wet(x,ir):return signal.fftconvolve(x,ir)[:N]

def fade(t0=EDIT,width=.04):
 t=np.arange(N)/SR;u=np.clip((t-t0)/width,0,1);return u*u*(3-2*u)

def octave(x):
 # Approximate octave-up through frequency bin remapping. A simple phase operation;
 # no identity-aware phase locking, so artifacts and transient smearing are expected.
 _,_,z=signal.stft(x,SR,nperseg=1024,noverlap=768,boundary='zeros',padded=True)
 dest=np.zeros_like(z);limit=(z.shape[0]-1)//2;src=z[1:limit+1];dest[2:2*limit+1:2]=np.abs(src)*np.exp(2j*np.angle(src))
 _,y=signal.istft(dest,SR,nperseg=1024,noverlap=768,input_onesided=True,boundary=True)
 return np.pad(y,(0,max(0,N-len(y))))[:N]

def freeze(x):
 # Loop a 0.4 s captured tail slice; smooth overlap at wrap with equal-power-like fades.
 L=int(.4*SR);start=int((EDIT-.45)*SR);clip=x[start:start+L].copy(); cross=int(.04*SR)
 # Crossfade the final 40ms into the first 40ms, then cycle with 40ms overlap.
 step=L-cross;out=np.zeros(N);weight=np.zeros(N)
 window=np.ones(L);w=np.sin(np.linspace(0,np.pi/2,cross))**2;window[:cross]=w;window[-cross:]=w[::-1]
 for pos in range(int(EDIT*SR)-cross,N,step):
  lo=max(0,pos);hi=min(N,pos+L);offset=lo-pos;out[lo:hi]+=clip[offset:offset+hi-lo]*window[offset:offset+hi-lo];weight[lo:hi]+=window[offset:offset+hi-lo]
 return np.divide(out,weight,out=np.zeros_like(out),where=weight>1e-8)

def spectral_separate(mixture):
 # Learn two templates from source-exclusive regions of the MIXTURE, not isolated stems.
 # This favourable setup is known and labelled; it is not blind semantic separation.
 f,t,z=signal.stft(mixture,SR,nperseg=2048,noverlap=1536,boundary='zeros',padded=True)
 mag=np.abs(z);ta=np.mean(mag[:,(t>.10)&(t<.48)],axis=1);tb=np.mean(mag[:,(t>1.72)&(t<2.12)],axis=1)
 basis=np.stack([ta,tb],axis=1)+1e-12;basis/=np.maximum(np.linalg.norm(basis,axis=0),1e-12)
 coeff=np.array([optimize.nnls(basis,mag[:,i])[0] for i in range(mag.shape[1])]).T
 estimates=basis[:,:,None]*coeff[None,:,:];mask=estimates[:,0,:]/np.maximum(estimates.sum(axis=1),1e-12)
 _,a=signal.istft(z*mask,SR,nperseg=2048,noverlap=1536,boundary=True);_,b=signal.istft(z*(1-mask),SR,nperseg=2048,noverlap=1536,boundary=True)
 return np.pad(a,(0,max(0,N-len(a))))[:N],np.pad(b,(0,max(0,N-len(b))))[:N]

def snr(ref,est):return 10*np.log10((np.sum(ref**2)+1e-20)/(np.sum((ref-est)**2)+1e-20))
def stereo(a,b,panA=-.45,panB=.45):
 return np.column_stack((a*np.sqrt((1-panA)/2)+b*np.sqrt((1-panB)/2),a*np.sqrt((1+panA)/2)+b*np.sqrt((1+panB)/2)))

def run(out):
 start=time.perf_counter(); out.mkdir(parents=True,exist_ok=True);ir=impulse();x=chord([60,64,67],0,1.6);y=chord([64,67,71],.6,2.25,phase=.32);mix=x+y
 A=wet(x,ir);B=wet(y,ir);f=fade(); pitched=octave(A);held=freeze(A);erase=A*(1-f); pitched_a=A*(1-f)+pitched*f;freeze_a=A*(1-f)+held*f
 # Onset baseline assigns ALL mixture input after event B's onset to B's state.
 split=int(.6*SR);old=mix.copy();old[split:]=0;new=mix-old;baselineA,baselineB=wet(old,ir),wet(new,ir)
 estA,estB=spectral_separate(mix);sepA,sepB=wet(estA,ir),wet(estB,ir)
 # For each example compare edits after identical source events; dry signals stop by 2.25s.
 outputs={'01_unedited':stereo(A,B),'02_selective_octave':stereo(pitched_a,B),'03_selective_erase':stereo(erase,B),'04_selective_freeze':stereo(freeze_a,B),
 '05_onset_baseline_erase':stereo(baselineA*(1-f),baselineB),'06_mixture_recovered_erase':stereo(sepA*(1-f),sepB),
 '07_source_mixture':np.column_stack((mix,mix)), '08_target_A_tail':stereo(A,np.zeros(N)), '09_other_B_tail':stereo(np.zeros(N),B)}
 # Independent changes in A's pan and decay after edit, via a stereo crossfade.
 pos=stereo(A,B);moved=stereo(A,B,panA=.85);outputs['10_selective_move']=pos*(1-f[:,None])+moved*f[:,None]
 dec=A*((1-f)+f*np.exp(-np.maximum(np.arange(N)/SR-EDIT,0)*2));outputs['11_selective_shorter_decay']=stereo(dec,B)
 maxpeak=max(np.max(np.abs(v)) for v in outputs.values());gain=.85/maxpeak
 for name,v in outputs.items():sf.write(out/(name+'.wav'),v*gain,SR,subtype='PCM_16')
 tail=slice(int(EDIT*SR),N); err_erase=stereo(erase,B)-stereo(np.zeros(N),B);err_leak=stereo(erase,B)-stereo(A*(1-f),B)
 metrics={'sample_rate':SR,'event_A_source_end_s':1.6,'event_B_source_end_s':2.25,'edit_time_s':EDIT,'fade_seconds':.04,'global_gain':float(gain),
 'baseline_event_A_source_snr_db':float(snr(x,old)), 'baseline_event_B_source_snr_db':float(snr(y,new)),
 'template_event_A_source_snr_db':float(snr(x,estA)), 'template_event_B_source_snr_db':float(snr(y,estB)),
 'template_mixture_reconstruction_snr_db':float(snr(mix,estA+estB)),
 'known_stem_erase_A_residual_rms_after_fade':float(np.sqrt(np.mean(erase[int((EDIT+.04)*SR):]**2))),
 'known_stem_B_unintended_change_max':float(np.max(np.abs(err_leak))),
 'baseline_A_tail_snr_after_edit_db':float(snr(A[tail],baselineA[tail])), 'baseline_B_tail_snr_after_edit_db':float(snr(B[tail],baselineB[tail])),
 'template_A_tail_snr_after_edit_db':float(snr(A[tail],sepA[tail])), 'template_B_tail_snr_after_edit_db':float(snr(B[tail],sepB[tail])),
 'numpy_event_tail_memory_bytes':int(A.nbytes+B.nbytes),'elapsed_seconds':time.perf_counter()-start}
 (out/'metrics.json').write_text(json.dumps(metrics,indent=2));print(json.dumps(metrics,indent=2))
 assert np.max(np.abs(err_leak))<1e-12 and np.max(np.abs(erase[int((EDIT+.04)*SR):]))<1e-12
 assert np.isfinite(np.array(list(metrics.values()))).all()
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--out',default='experiments/rewrite-003/renders');args=p.parse_args();run(Path(args.out))
