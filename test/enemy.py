# Judges the hunter from recorded positions and headings, not the ROM's own logic.
import sys,math
import numpy as np
R=256*240+40;raw=np.fromfile(sys.argv[1],dtype=np.uint8);N=len(raw)//R
st=raw.reshape(N,R)[:,256*240:].astype(int)
w=lambda v:((v+2048)%4096)-2048
rows=[]
for t,s in enumerate(st):
    cam=[(s[i+2]<<8|s[i+1]) for i in (0,3,6)]
    h=[s[25+2*i]<<8|s[24+2*i] for i in range(3)]
    d=np.array([w(c-p) for c,p in zip(cam,h)],float)
    a,b=2*math.pi*s[30]/256,2*math.pi*s[31]/256
    f=np.array([math.sin(a)*math.cos(b),-math.sin(b),math.cos(a)*math.cos(b)])
    dist=np.linalg.norm(d); off=math.degrees(math.acos(max(-1,min(1,f@d/max(dist,1)))))
    rows.append((t,dist,off,s[33],s[34],s[23],s[21]&1,s[35],s[32]))
a=np.array(rows)[60:]
live=a[a[:,6]>0]
print('distance min %.0f max %.0f | nose error start %.0f deg, median %.0f deg'%(live[:,1].min(),live[:,1].max(),a[0,2],np.median(a[:,2])))
print('shots %d, bolts that hit %d, final shields %d, hunter alive %d'%(a[-1,3],a[-1,4],a[-1,5],a[-1,6]))
sh=np.nonzero(np.diff(a[:,3])>0)[0]
if len(sh): print('nose error when firing: worst %.1f deg, distance %.0f..%.0f'%(a[sh,2].max(),a[sh,1].min(),a[sh,1].max()))
print('time in break-away %.0f%%'%(100*(a[:,7]>0).mean()))
