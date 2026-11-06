# Independent floating-point model of the camera, ships, and clipping.
import sys,math,re
import numpy as np
from PIL import Image,ImageDraw
from scipy import ndimage
src=open('tables.inc').read()
def tab(n):
    m=re.search(rf'^{n}:\n((?:  \.byte .*\n)+)',src,re.M); return [int(x) for l in m.group(1).split('\n') if l for x in l.replace('.byte','').split(',')]
s8=lambda v:v-256 if v>127 else v
V=[(s8(a^128),s8(b^128),s8(c^128)) for a,b,c in zip(tab('vxu'),tab('vyu'),tab('vzu'))]
ea,eb,em=tab('ea'),tab('eb'),tab('emask'); fa,fb,fc=tab('fa'),tab('fb'),tab('fc')
SHIP=[(0,0,700,0,0),(-500,150,1400,90,60),(650,-200,1900,170,120),(300,400,-1200,40,200)]
def ang(a):return 2*math.pi*a/256
def rot_view(y,p):
    cy,sy,cp,sp=math.cos(ang(y)),math.sin(ang(y)),math.cos(ang(p)),math.sin(ang(p))
    return np.array([[cy,0,-sy],[-sy*sp,cp,-cy*sp],[sy*cp,sp,cy*cp]])
def rot_ship(y,p):
    ca,sa,cb,sb=math.cos(ang(y)),math.sin(ang(y)),math.cos(ang(p)),math.sin(ang(p))
    return np.array([[ca,sa*sb,sa*cb],[0,cb,-sb],[-sa,ca*sb,ca*cb]])
NEAR=128
def proj(c): return (96+c[0]*127/c[2],64-c[1]*127/c[2])
def render(st):
    cam=[ (int(st[i+2])<<8|int(st[i+1]))+st[i]/256 for i in (0,3,6)]
    cy,cp=int(st[9]),int(st[10]); ty,tp=int(st[12]),int(st[13])
    Vm=rot_view(cy,cp); im=Image.new('1',(192,128)); d=ImageDraw.Draw(im); cnt=0
    for si,(sx,sy,sz,oy,op) in enumerate(SHIP):
        if not int(st[21])>>si&1: continue
        if si==0:
            sx,sy,sz=[(int(st[25+2*i])<<8|int(st[24+2*i])) for i in range(3)]
        delta=[((p-c+2048)%4096)-2048 for p,c in zip((sx,sy,sz),cam)]
        rel=Vm@np.array(delta,float)
        if rel[2]+100<0: continue
        M=Vm@(rot_ship(int(st[30]),int(st[31])) if si==0 else rot_ship(ty+oy,tp+op))
        lod=rel[2]>=1024
        vr=range(10,15) if lod else range(10)
        fr=range(7,12) if lod else range(7)
        er=range(15,23) if lod else range(15)
        cv={i:rel+M@np.array(V[i],float) for i in vr}
        if abs(rel[0])>rel[2]+100 or abs(rel[1])>rel[2]+100: continue
        eye=-(M.T@rel)
        vis=0
        for i,fi in enumerate(fr):
            A,B,C=[np.array(V[k],float) for k in (fa[fi],fb[fi],fc[fi])]
            n=np.cross(B-A,C-A)
            if n@(eye-A)>0: vis|=1<<i
        for ei in er:
            a,b,m=ea[ei],eb[ei],em[ei]
            if not m&vis: continue
            p,q=cv[a],cv[b]
            if p[2]<NEAR and q[2]<NEAR: continue
            if p[2]<NEAR or q[2]<NEAR:
                t=(NEAR-p[2])/(q[2]-p[2]); r=p+(q-p)*t
                if p[2]<NEAR:p=r
                else:q=r
            P,Q=proj(p),proj(q)
            # clip to the view with a dense, simple parametric test
            x0,y0,x1,y1=P[0],P[1],Q[0],Q[1]
            t0,t1=0,1;okk=True
            for dp,lo,hi,v in ((x1-x0,0,191,x0),(y1-y0,1,126,y0)):
                if abs(dp)<1e-9:
                    if v<lo or v>hi: okk=False
                    continue
                a0,a1=sorted(((lo-v)/dp,(hi-v)/dp)); t0=max(t0,a0);t1=min(t1,a1)
            if not okk or t0>t1: continue
            d.line([(x0+(x1-x0)*t0,y0+(y1-y0)*t0),(x0+(x1-x0)*t1,y0+(y1-y0)*t1)],fill=1); cnt+=1
    return np.array(im,bool),cnt
raw=np.fromfile(sys.argv[1],dtype=np.uint8);R=256*240+40;N=len(raw)//R
f=raw.reshape(N,R); imgs=f[:,:256*240].reshape(N,240,256); st=f[:,256*240:]
skip=int(sys.argv[2]) if len(sys.argv)>2 else 90
ox,oy=32,int(sys.argv[3]) if len(sys.argv)>3 else 25
S=np.ones((5,5),bool); res=[]
outside=0
for t in range(skip,N,3):
    w=imgs[t].astype(bool); win=w[oy:oy+128,ox:ox+192].copy(); win[112:]=False  # score row excluded
    cands={}
    for k in range(0,20):
        key=st[t-k][:14].tobytes()+st[t-k][21:22].tobytes()+st[t-k][24:32].tobytes(); cands[key]=st[t-k]
    best=None
    for s in cands.values():
        e,_=render(s); e=e.copy(); e[112:]=False
        if e.sum()==0 and win.sum()==0: best=(1.0,1.0);break
        a=(win&ndimage.binary_dilation(e,S)).sum()/max(1,win.sum())
        b=(e&ndimage.binary_dilation(win,S)).sum()/max(1,e.sum())
        if best is None or min(a,b)>min(best): best=(a,b)
    res.append((min(best),t))
res.sort()
v=np.array([r[0] for r in res])
print('frames',len(v),'mean match',round(v.mean(),4),'min',round(v.min(),3),'<0.95:',int((v<.95).sum()),'<0.8:',int((v<.8).sum()),'pixels outside view',outside)
print('worst',[(round(a,2),t) for a,t in res[:6]])
if len(sys.argv)>4:
    t=int(sys.argv[4]); w=imgs[t].astype(bool)[oy:oy+128,ox:ox+192]; print('rom px',w.sum(),'stats',list(st[t][:14]))
    e,c=render(st[t]); print('ref px',e.sum(),'edges',c)
    out=np.zeros((128,192,3),np.uint8); out[...,0]=w*255; out[...,1]=e*255
    Image.fromarray(out).resize((576,384),0).save('/tmp/cmp.png')
if len(sys.argv)>5:
    t=int(sys.argv[4]); w=imgs[t].astype(bool)[oy:oy+128,ox:ox+192]; best=(-1,None,0)
    for k in range(0,20):
        e,_=render(st[t-k])
        a=(w&ndimage.binary_dilation(e,S)).sum()/max(1,w.sum()); b=(e&ndimage.binary_dilation(w,S)).sum()/max(1,e.sum())
        if min(a,b)>best[0]: best=(min(a,b),e,k)
    print('best',best[0],'k',best[2]); e=best[1]
    out=np.zeros((128,192,3),np.uint8); out[...,0]=w*255; out[...,1]=e*255
    Image.fromarray(out).resize((576,384),0).save('/tmp/cmp.png')
