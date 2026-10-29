"""Download the pinned model; verify integrity before handing it to PyTorch."""
import hashlib, pathlib, urllib.request
from render import SHA, URL
p=pathlib.Path(__file__).parent/'models'/'freesoundloop10k-default.ts'
p.parent.mkdir(parents=True,exist_ok=True)
tmp=p.with_suffix('.download')
print('Downloading approximately 150 MB from the model publisher...')
urllib.request.urlretrieve(URL,tmp)
hash_value=hashlib.sha256(tmp.read_bytes()).hexdigest()
if hash_value!=SHA:
 tmp.unlink(missing_ok=True)
 raise SystemExit('SHA-256 mismatch; download discarded')
tmp.replace(p)
print('Verified:',p,'SHA-256:',hash_value)
