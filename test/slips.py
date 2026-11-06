import numpy as np,sys
raw=np.fromfile(sys.argv[1],dtype=np.uint8);R=256*240+40;N=len(raw)//R
f=raw.reshape(N,R)
im=f[:,:256*240].reshape(N,240,256)
mode=f[:,256*240+19]
k=mode==1
k[:]=k&np.r_[k[8:],[False]*8]&np.r_[[False]*8,k[:-8]]  # flight frames only, away from mode changes
im=im[k]
cov=im[:,27,72:183].sum(1)
print(int((cov<60).sum()),'/',len(im))
