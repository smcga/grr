"""Known-identity GHOST -> moving ROOM -> REWRITE selective tail edit."""
import argparse, importlib.util, json, pathlib, subprocess, tempfile, time
import numpy as np, soundfile as sf
from scipy import signal
SR=48000; N=8*SR; EDIT=3.2

def module(path):
 spec=importlib.util.spec_from_file_location('rewrite_experiment',path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def prop(exe,src,temp,name):
 source=temp/(name+'.bin');dest=temp/(name+'-out.bin');np.asarray(src,dtype='<f8').tofile(source)
 subprocess.run([str(exe),str(source),str(dest)],check=True)
 out=np.fromfile(dest,dtype='<f8');assert out.size==2*N;return out.reshape(-1,2)

def run(exe,ghost,out):
 start=time.perf_counter();out.mkdir(parents=True,exist_ok=True)
 r=module(pathlib.Path(__file__).resolve().parent.parent/'rewrite-003'/'render.py')
 desc,rate=sf.read(ghost,dtype='float64');assert rate==SR and desc.ndim==1
 a=np.zeros(N);a[:min(len(desc),N)]=desc[:N]; b24=r.chord([64,67,71],.6,2.25,.32);b=signal.resample_poly(b24,2,1)[:N]
 with tempfile.TemporaryDirectory(dir=out) as tempstr:
  temp=pathlib.Path(tempstr);A=prop(exe,a,temp,'a');B=prop(exe,b,temp,'b');shared=prop(exe,a+b,temp,'shared')
 diff=float(np.max(np.abs(shared-A-B))); assert diff<1e-10
 t=np.arange(N)/SR;u=np.clip((t-EDIT)/.04,0,1);f=(u*u*(3-2*u))[:,None]
 erased=A*(1-f)+B
 # Operate per channel on downsampled 24 kHz labelled tail using REWRITE's octave function.
 p24=[]
 for ch in range(2):
  q=signal.resample_poly(A[:,ch],1,2);up=signal.resample_poly(r.octave(q),2,1);p24.append(up[:N])
 octave=np.stack(p24,axis=1);pitched=A*(1-f)+octave*f+B
 outputs={'01_shared_room_unedited':shared,'02_erase_ghost_tail':erased,'03_octave_ghost_tail':pitched,'04_ghost_room_contribution':A,'05_other_room_contribution':B,'06_ghost_descendant_input':np.column_stack((a,a))}
 peak=max(float(np.max(np.abs(v))) for v in outputs.values());gain=.85/max(peak,1e-20)
 for name,v in outputs.items():sf.write(out/(name+'.wav'),v*gain,SR,subtype='PCM_16')
 remaining=float(np.max(np.abs((A*(1-f))[int((EDIT+.04)*SR):])));assert remaining<1e-12
 metrics={'sample_rate':SR,'ghost_source_end_s':len(desc)/SR,'other_source_end_s':2.25,'room_motion_start_s':2.8,'room_motion_end_s':3.8,'edit_start_s':EDIT,'superposition_max_abs_error':diff,'ghost_remaining_after_erase_max':remaining,'other_component_change_max':float(np.max(np.abs((erased-A*(1-f))-B))),'global_gain':gain,'elapsed_s':time.perf_counter()-start,'ghost_input':str(ghost),'event_room_memory_bytes':int(A.nbytes+B.nbytes)}
 (out/'metrics.json').write_text(json.dumps(metrics,indent=2),encoding='utf-8');print(json.dumps(metrics,indent=2))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--room-exe',required=True,type=pathlib.Path);p.add_argument('--ghost',required=True,type=pathlib.Path);p.add_argument('--out',type=pathlib.Path,default=pathlib.Path('experiments/convergence-004/renders'));a=p.parse_args();run(a.room_exe.resolve(),a.ghost,a.out)
