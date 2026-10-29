import importlib.util, pathlib
import numpy as np
p=pathlib.Path(__file__).with_name('render.py');spec=importlib.util.spec_from_file_location('rewrite',p);r=importlib.util.module_from_spec(spec);spec.loader.exec_module(r)
x=r.chord([60,64,67],0,1.6);y=r.chord([64,67,71],.6,2.25,.32);assert np.linalg.norm(x[int(.7*r.SR):int(1.4*r.SR)])>0 and np.linalg.norm(y[int(.7*r.SR):int(1.4*r.SR)])>0
A,B=r.spectral_separate(x+y);assert r.snr(x+y,A+B)>100;assert r.snr(x,A)<30, 'hard case should retain measurable attribution error'
t=np.arange(r.N)/r.SR;tone=np.sin(2*np.pi*375*t);out=r.octave(tone);s=out[r.SR:2*r.SR];peak=np.argmax(np.abs(np.fft.rfft(s*np.hanning(len(s)))));assert abs(peak-750)<=3
ir=r.impulse();a=r.wet(x,ir);b=r.wet(y,ir);combined=r.wet(x+y,ir);assert np.max(np.abs(combined-a-b))<1e-10
f=r.fade();erased=a*(1-f);assert np.max(np.abs(erased[int((r.EDIT+.04)*r.SR):]))<1e-12
print('overlap, additive propagation, mixture reconstruction, attribution error, selective erase, octave test: passed')
