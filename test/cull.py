# Face culling error against exact maths, by distance. Run on a one-ship ROM with VIS=1.
import sys
path=sys.argv[1]; sys.argv=['x',path,'99999']
exec(open('test/ref.py').read().split("raw=np.fromfile")[0])
R=256*240+40;raw=np.fromfile(path,dtype=np.uint8);N=len(raw)//R
st=raw.reshape(N,R)[:,256*240:].astype(int)
seen=set();rows=[]
for s in st[60:]:
    if not s[21]&1: continue
    k=tuple(s[:14])+tuple(s[24:32])
    if k in seen: continue
    seen.add(k)
    cam=[(s[i+2]<<8|s[i+1])+s[i]/256 for i in (0,3,6)]
    hp=[s[25+2*i]<<8|s[24+2*i] for i in range(3)]
    Vm=rot_view(s[9],s[10]);M=Vm@rot_ship(s[30],s[31])
    rel=Vm@np.array([((p-c+2048)%4096)-2048 for p,c in zip(hp,cam)],float)
    d=np.linalg.norm(rel)
    if rel[2]+100<0 or abs(rel[0])>rel[2]+100 or abs(rel[1])>rel[2]+100: continue  # culled: its stat is stale
    fr=range(7,12) if rel[2]>=1024 else range(7)
    eye=-(M.T@rel);m=0;hs=[]
    for i,fi in enumerate(fr):
        A,B,C=[np.array(V[q],float) for q in (fa[fi],fb[fi],fc[fi])]
        n=np.cross(B-A,C-A);n/=np.linalg.norm(n);h=n@(eye-A);hs.append(h)
        if h>0:m|=1<<i
    x=m^int(s[15])
    rows.append((d,bin(x).count('1')>0,max([abs(h) for i,h in enumerate(hs) if x>>i&1] or [0])))
a=np.array(rows)
for lo,hi in ((130,300),(300,600),(600,1024),(1024,5000)):
    q=a[(a[:,0]>=lo)&(a[:,0]<hi)]
    if len(q): print(f'  dist {lo}-{hi}: {len(q)} states, wrong {int(q[:,1].sum())}, worst plane error {q[:,2].max():.1f}')
