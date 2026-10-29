"""GHOST-002: offline RAVE descendants. No learned model is shipped here."""
import argparse, hashlib, json, math, os, time
from pathlib import Path
import numpy as np
import soundfile as sf
import torch
from scipy import signal

SR=48000
SEED=20261029
SHA='0875195056b2990d411a6b38e63c5ea591d2df9b9bd4ac2c25c06a65c9e16de1'
URL='https://huggingface.co/Tangible-Music-Lab/RAVE_models/resolve/3bf275ff6e223ddf9e82aacc5e9933f4af401fe8/freesoundloop10k_default_b2048_r48000_z16.ts'

def envelope(n):
 t=np.arange(n)/SR
 return np.minimum(1,t/.012)*np.exp(-2*t)

def melodic():
 n=int(2.4*SR); out=np.zeros(n,dtype=np.float64)
 for j,midi in enumerate([60,64,67,72,67,64]):
  start=int(j*.34*SR); length=int(.28*SR);t=np.arange(length)/SR;f=440*2**((midi-69)/12)
  wave=sum(np.sin(2*np.pi*f*k*t)/k**1.4 for k in range(1,8));out[start:start+length]+=.28*envelope(length)*wave
 return out

def drums():
 rng=np.random.default_rng(SEED); out=np.zeros(int(2.4*SR))
 for j,startsec in enumerate([0,.28,.5,.75,1.,1.28,1.52,1.75]):
  length=int(.18*SR);t=np.arange(length)/SR; noise=rng.standard_normal(length);f=95 if j%3==0 else 230
  wave=.28*np.sin(2*np.pi*f*t-120*t*t)*np.exp(-t*32)+.22*noise*np.exp(-t*35);start=int(startsec*SR);out[start:start+length]+=wave
 return out

def references():
 # Synthetic latent targets, not actual recordings or guaranteed instrument labels.
 n=int(2.4*SR);t=np.arange(n)/SR; rng=np.random.default_rng(SEED+10)
 metal=np.sin(2*np.pi*360*t)+.5*np.sin(2*np.pi*997*t)+.25*np.sin(2*np.pi*2170*t)
 voice=sum(np.sin(2*np.pi*110*k*t)*np.exp(-((110*k-700)/450)**2)/k**.5 for k in range(1,17))
 noise=signal.sosfilt(signal.butter(2,[350,4000],btype='bandpass',fs=SR,output='sos'),rng.standard_normal(n))
 return {'metal':.12*metal,'voice':.2*voice,'noise':.18*noise}

def load(path):
 m=torch.jit.load(str(path),map_location='cpu');m.eval();return m

def encode_style(path, x):
 # Fresh model each pass: cached streaming state cannot leak between examples.
 torch.manual_seed(SEED);m=load(path)
 with torch.inference_mode(): z=m.encode(torch.tensor(x,dtype=torch.float32).view(1,1,-1))
 return z.mean(-1,keepdim=True),z.std(-1,keepdim=True).clamp(min=.01)

def transform(path,x,style,mutation,inheritance,seed):
 start=time.perf_counter();torch.manual_seed(seed);m=load(path)
 with torch.inference_mode():
  z=m.encode(torch.tensor(x,dtype=torch.float32).view(1,1,-1)); mu=z.mean(-1,keepdim=True);std=z.std(-1,keepdim=True).clamp(min=.01)
  target=(z-mu)/std*style[1]+style[0];zz=(1-mutation)*z+mutation*target
  decoded=m.decode(zz).view(-1).cpu().numpy().astype(np.float64)
 # Streaming model drops a partial frame and has unmeasured look-ahead/latency.
 decoded=np.pad(decoded,(0,max(0,len(x)-len(decoded))))[:len(x)]
 # Inheritance is explicitly a waveform blend, not guaranteed pitch preservation.
 result=inheritance*x+(1-inheritance)*decoded
 return result,time.perf_counter()-start

def pack(seed, seq):
 # Each generation occupies an equal 2.4 s slot; no overlap, so edits can be inspected.
 return np.concatenate([seed]+seq)

def matched(seed, tracks):
 target=np.sqrt(np.mean(seed*seed))
 return [v*min(4.0,target/max(np.sqrt(np.mean(v*v)),1e-12)) for v in tracks]

def write(path,x,scale):
 sf.write(path,np.clip(x*scale,-.98,.98),SR,subtype='PCM_16')

def source_metrics(source, out, melodic_input):
    # Approximate descriptors, not semantic instrument classification or perceptual evaluation.
    frame,hop=1024,512
    def contours(x):
        frames=np.lib.stride_tricks.sliding_window_view(x,frame)[::hop]
        return np.sqrt(np.mean(frames*frames,axis=1))
    a,b=contours(source),contours(out)
    result={"rms_envelope_correlation": float(np.corrcoef(a,b)[0,1]) if np.std(b)>1e-12 else None}
    # Best envelope alignment within +/- 500 ms, with overlap re-normalized.
    best=(-2,0)
    for lag in range(-int(.5*SR/hop),int(.5*SR/hop)+1):
        aa=a[max(0,-lag):min(len(a),len(a)-lag)];bb=b[max(0,lag):min(len(b),len(b)+lag)]
        if len(aa)>2 and np.std(aa)>1e-12 and np.std(bb)>1e-12:
            corr=float(np.corrcoef(aa,bb)[0,1])
            if corr>best[0]:best=(corr,lag)
    result["best_envelope_correlation_within_500ms"]=best[0]
    result["best_envelope_lag_ms"]=round(best[1]*hop/SR*1000,2)
    def centroid(x):
        spec=np.abs(np.fft.rfft(x*np.hanning(len(x))))
        return float(np.sum(np.fft.rfftfreq(len(x),1/SR)*spec)/max(np.sum(spec),1e-12))
    result["spectral_centroid_hz"]=centroid(out)
    if melodic_input:
        estimates=[]
        for j,midi in enumerate([60,64,67,72,67,64]):
            start=int((j*.34+.07)*SR);length=int(.16*SR); x=out[start:start+length]
            if len(x)<length:continue
            mag=np.abs(np.fft.rfft(x*np.hanning(length),n=32768));freq=np.fft.rfftfreq(32768,1/SR)
            # Strongest local bin near expected note: sensitive to harmonics, not a full pitch tracker.
            expected=440*2**((midi-69)/12); ix=np.where((freq>expected*.8)&(freq<expected*1.25))[0]
            f=freq[ix[np.argmax(mag[ix])]];estimates.append(abs(1200*np.log2(f/expected)))
        result["median_known_note_local_peak_error_cents"]=float(np.median(estimates))
    return result

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--model',required=True);ap.add_argument('--out',default='experiments/ghost-002/renders');ap.add_argument('--mutation',type=float,default=.65);ap.add_argument('--inheritance',type=float,default=.30);ap.add_argument('--evolution',choices=['chain','root'],default='chain');ap.add_argument('--identity',nargs=3,choices=['metal','voice','noise'],default=['metal','voice','noise']);ap.add_argument('--input',help='optional mono/stereo WAV; replaces the synthetic melodic source, no automatic loudness normalization');args=ap.parse_args()
 if not 0<=args.mutation<=1 or not 0<=args.inheritance<=1:ap.error('mutation and inheritance must be in [0,1]')
 path=Path(args.model);digest=hashlib.sha256(path.read_bytes()).hexdigest()
 if digest!=SHA:raise ValueError('Unexpected checkpoint SHA-256; inspect source and use documented pinned model')
 torch.set_num_threads(4);torch.manual_seed(SEED);out=Path(args.out);out.mkdir(parents=True,exist_ok=True)
 styles={name:encode_style(path,x) for name,x in references().items()};print('encoded target statistics:',', '.join(styles))
 records={}; outputs={}
 custom=None
 if args.input:
  custom,rate=sf.read(args.input,dtype='float64');custom=custom.mean(axis=1) if custom.ndim==2 else custom
  if rate!=SR:custom=signal.resample_poly(custom,SR//math.gcd(SR,rate),rate//math.gcd(SR,rate))
  if len(custom)<4096:raise ValueError('input must be at least 4096 samples')
 for label,seed in [('melodic' if custom is None else 'custom',melodic() if custom is None else custom),('percussive',drums())]:
  descendants=[];cur=seed.copy();timings=[]
  for i,name in enumerate(args.identity):
   src=cur if args.evolution=='chain' else seed
   cur,secs=transform(path,src,styles[name],args.mutation*(i+1)/3,args.inheritance,SEED+100+i);descendants.append(cur);timings.append(secs)
  # Conventional comparator: progressively damp each successive repeat, no neural inference.
  base=[];v=seed.copy()
  for cutoff in [6500,3000,1400]:v=signal.sosfilt(signal.butter(3,cutoff,fs=SR,output='sos'),v);base.append(v.copy())
  root=[]
  for i,name in enumerate(args.identity):root.append(transform(path,seed,styles[name],args.mutation*(i+1)/3,args.inheritance,SEED+100+i)[0])
  outputs[label+('_neural_lineage' if args.evolution=='chain' else '_neural_selected_root')]=pack(seed,descendants);outputs[label+'_neural_from_root']=pack(seed,root);outputs[label+'_baseline']=pack(seed,base)
  outputs[label+'_neural_matched']=pack(seed,matched(seed,descendants));outputs[label+'_baseline_matched']=pack(seed,matched(seed,base))
  for i,v in enumerate(descendants):outputs[label+f'_descendant_{i+1}']=v
  outputs[label+'_source']=seed
  records[label]={'times_s':timings,'source_peak':float(np.max(np.abs(seed))),'descendant_rms':[float(np.sqrt(np.mean(v*v))) for v in descendants], 'descendant_source_correlation':[float(np.corrcoef(seed,v)[0,1]) for v in descendants], 'source_descriptors':source_metrics(seed,seed,label=='melodic'), 'neural_descriptors':[source_metrics(seed,v,label=='melodic') for v in descendants], 'baseline_descriptors':[source_metrics(seed,v,label=='melodic') for v in base]}
 # One global scale across all outputs: relative levels are preserved; no clipping.
 maxpeak=max(float(np.max(np.abs(v))) for v in outputs.values());scale=.85/maxpeak if maxpeak else 1
 for name,x in outputs.items():write(out/(name+'.wav'),x,scale)
 info={'seed':SEED,'samplerate':SR,'checkpoint_sha256':digest,'checkpoint_url':URL,'model':'Tangible Music Lab RAVE freesoundloop10k default','torch':torch.__version__,'device':'CPU','mutation':args.mutation,'inheritance_waveform_mix':args.inheritance,'evolution':args.evolution,'identity_order':args.identity,'global_gain':scale,'metrics':records}
 (out/'metrics.json').write_text(json.dumps(info,indent=2),encoding='utf-8');print(json.dumps(info,indent=2))
if __name__=='__main__':main()
