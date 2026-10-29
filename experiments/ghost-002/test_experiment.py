import importlib.util, pathlib, sys
import numpy as np, torch
p=pathlib.Path(__file__).with_name('render.py');spec=importlib.util.spec_from_file_location('ghost_render',p);r=importlib.util.module_from_spec(spec);spec.loader.exec_module(r)
model=pathlib.Path(sys.argv[1]); torch.set_num_threads(4)
x=r.melodic(); style=r.encode_style(model,r.references()['metal'])
a,_=r.transform(model,x,style,.5,.3,r.SEED);b,_=r.transform(model,x,style,.5,.3,r.SEED)
assert len(a)==len(x) and np.isfinite(a).all()
assert np.array_equal(a,b), 'same seed and fresh model should reproduce exactly on this platform'
copy,_=r.transform(model,x,style,.5,1.,r.SEED); assert np.array_equal(copy,x)
no_style,_=r.transform(model,x,style,0,0,r.SEED); styled,_=r.transform(model,x,style,.8,0,r.SEED)
diff=np.sqrt(np.mean((no_style-styled)**2));assert diff>1e-5
print('deterministic fresh-model render, inheritance endpoints, latent mutation: passed; mutation RMS difference',diff)
