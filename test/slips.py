# Picture-timing check for DEBUG builds: the fixed marker line (picture y=2,
# x 40-150) must sit on the same screen row in every flight frame. Prints
# frames off the usual row, frames with no marker, and the usual row.
import numpy as np,sys
raw=np.fromfile(sys.argv[1],dtype=np.uint8);R=256*240+40;N=len(raw)//R
f=raw.reshape(N,R)
im=f[:,:256*240].reshape(N,240,256)
mode=f[:,256*240+19]
k=mode==1
k[:]=k&np.r_[k[8:],[False]*8]&np.r_[[False]*8,k[:-8]]  # flight frames only, away from mode changes
im=im[k]
cov=im[:,10:60,72:183].sum(2)            # rows 10-59
row=np.where(cov.max(1)>=100,cov.argmax(1)+10,-1)
good=np.bincount(row[row>=0]).argmax() if (row>=0).any() else -1
print(int(((row!=good)&(row>=0)).sum()),'off-row,',int((row<0).sum()),'missing, of',len(im),'frames; row',good)
