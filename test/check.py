import sys,math,collections
import numpy as np
from scipy import ndimage
raw=np.fromfile(sys.argv[1],dtype=np.uint8); N=len(raw)//(256*240+40)
fr=raw.reshape(N,256*240+40); 
imgs=fr[:,:256*240].reshape(N,240,256); st=fr[:,256*240:]
imgs=imgs[60:]; st=st[60:]
rows=collections.Counter(); comps=collections.Counter(); bad=0; yr=[]
for im in imgs:
    white=im.copy()
    # marker line: longest row of white
    r=white.sum(1); rows[int(r.argmax())]+=1
    y,x=np.nonzero(white); yr.append((y.min(),y.max(),x.min(),x.max()))
    l,n=ndimage.label(white,structure=np.ones((3,3))); comps[n]+=1
print('marker row',rows.most_common(5)); print('components',sorted(comps.items()))
a=np.array(yr); print('y',a[:,0].min(),a[:,1].max(),'x',a[:,2].min(),a[:,3].max())
