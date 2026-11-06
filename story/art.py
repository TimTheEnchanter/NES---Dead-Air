#!/usr/bin/env python3
"""Portraits drawn as code. Each is 48x48, colours 0-3: black, dark, mid, light."""
import math
W=H=48
def blank(): return [[0]*W for _ in range(H)]
def px(g,x,y,c):
    if 0<=x<W and 0<=y<H: g[y][x]=c
def ell(g,cx,cy,rx,ry,c,fn=None):
    for y in range(H):
        for x in range(W):
            if ((x-cx)/rx)**2+((y-cy)/ry)**2<=1 and (fn is None or fn(x,y)): g[y][x]=c
def rect(g,x0,y0,x1,y1,c):
    for y in range(y0,y1+1):
        for x in range(x0,x1+1): px(g,x,y,c)
def line(g,x0,y0,x1,y1,c):
    n=max(abs(x1-x0),abs(y1-y0),1)
    for i in range(n+1): px(g,round(x0+(x1-x0)*i/n),round(y0+(y1-y0)*i/n),c)
def poly(g,pts,c):
    for y in range(H):
        xs=[]
        for (x0,y0),(x1,y1) in zip(pts,pts[1:]+pts[:1]):
            if (y0<=y<y1) or (y1<=y<y0): xs.append(x0+(y-y0)*(x1-x0)/(y1-y0))
        xs.sort()
        for a,b in zip(xs[::2],xs[1::2]):
            for x in range(round(a),round(b)+1): px(g,x,y,c)
def outline(g,src,c):
    """Dark edge around everything that is not background."""
    out=[r[:] for r in g]
    for y in range(H):
        for x in range(W):
            if g[y][x]==0 and any(0<=x+dx<W and 0<=y+dy<H and g[y+dy][x+dx]!=0 for dx,dy in ((1,0),(-1,0),(0,1),(0,-1))):
                out[y][x]=c
    return out
def uniform(g,stripe=True):
    # shoulders and flight-suit collar
    poly(g,[(2,47),(4,40),(15,35),(22,33),(26,33),(33,35),(44,40),(46,47)],2)
    poly(g,[(17,34),(24,40),(31,34),(30,31),(18,31)],1)
    line(g,14,36,20,47,1); line(g,34,36,28,47,1)
    if stripe:
        rect(g,37,41,42,42,3); rect(g,37,44,42,45,3)
def face_base(g,cx=24,top=9,jaw=31,wide=11):
    rect(g,cx-5,29,cx+5,35,2)         # neck, behind the chin
    ell(g,cx,20,wide,13,3)
    poly(g,[(cx-wide,20),(cx-wide+1,27),(cx-5,jaw),(cx+5,jaw),(cx+wide-1,27),(cx+wide,20)],3)
    # shading down the right of the face
    for y in range(14,30):
        edge=cx+wide-1-abs(y-21)//3
        for x in range(edge-1,edge+3):
            if g[y][x]==3 and (x>=edge+1 or (x+y)%2==0): g[y][x]=2
    px(g,cx-wide-1,21,2); px(g,cx-wide-1,22,2); px(g,cx+wide+1,21,2); px(g,cx+wide+1,22,2)
def eye(g,x,y,lid=False):
    rect(g,x,y,x+3,y,1)               # upper lid
    px(g,x+1,y+1,1); px(g,x+2,y+1,1); px(g,x+1,y+2,1); px(g,x+2,y+2,1)  # iris
    if lid: rect(g,x,y+1,x+3,y+1,1)   # heavier lid
def nose(g,cx=24,y0=20,y1=25):
    for y in range(y0,y1): px(g,cx+1,y,2)
    line(g,cx-1,y1,cx+2,y1,2)
def hale(expr):
    g=blank(); uniform(g); face_base(g,wide=11)
    # short dark hair, swept up at the front
    poly(g,[(12,19),(12,11),(16,6),(24,4),(32,5),(36,10),(36,19),(34,15),(31,13),(24,14),(17,12),(14,15)],1)
    for x,y in ((20,7),(24,6),(28,7),(31,9),(17,9)): px(g,x,y,2); px(g,x+1,y,2)
    if expr=='smile':
        line(g,15,17,20,16,1); line(g,28,16,33,17,1)
        eye(g,16,19); eye(g,29,19); nose(g)
        line(g,18,27,20,28,1); rect(g,20,28,28,28,1); line(g,28,28,30,27,1)
        px(g,17,26,2); px(g,31,26,2); rect(g,22,29,26,29,2)
    else:
        line(g,15,17,20,18,1); line(g,28,18,33,17,1)
        eye(g,16,20); eye(g,29,20); nose(g)
        rect(g,20,28,28,28,1)
    # stubble
    for x,y in ((17,29),(20,30),(23,30),(26,30),(29,29),(16,27),(32,27)): px(g,x,y,2)
    return outline(g,None,1)
def voss(expr):
    g=blank(); uniform(g,False); face_base(g,wide=10,jaw=32)
    # receding grey hair: mid colour, close cropped
    poly(g,[(13,20),(13,11),(17,6),(24,5),(31,6),(35,11),(35,20),(34,14),(31,10),(27,9),(21,9),(17,10),(14,14)],2)
    for x in range(14,35,3): px(g,x,9+(x%2),1)
    # officer's collar pips and braid
    rect(g,8,41,13,42,3); rect(g,35,41,40,42,3); rect(g,11,44,13,45,3); rect(g,35,44,37,45,3)
    if expr=='hard':
        line(g,15,18,21,19,1); line(g,27,19,33,18,1); eye(g,16,20,True); eye(g,29,20,True)
        rect(g,20,28,28,28,1); line(g,19,28,19,29,1); line(g,29,28,29,29,1)
    else:
        line(g,15,17,21,17,1); line(g,27,17,33,17,1); eye(g,16,19); eye(g,29,19)
        rect(g,20,28,28,28,1)
    nose(g,24,19,25)
    # lines of age and a scar over the left eye
    line(g,18,24,17,27,2); line(g,30,24,31,27,2); line(g,21,12,27,12,2); line(g,18,16,20,18,2)
    return outline(g,None,1)
def mara(expr):
    g=blank(); uniform(g); face_base(g,wide=10,jaw=31)
    # hair pulled back, one strand loose, and a scar through the brow
    poly(g,[(13,20),(13,10),(18,5),(25,4),(32,6),(35,11),(35,20),(34,14),(29,11),(22,11),(17,13),(14,16)],2)
    poly(g,[(30,7),(38,5),(41,9),(38,13),(34,11)],2)
    line(g,14,16,13,24,2)
    if expr=='hard':
        line(g,16,18,21,19,1); line(g,27,19,32,18,1); eye(g,16,20,True); eye(g,29,20,True)
        rect(g,21,28,27,28,1)
    else:
        line(g,16,17,21,17,1); line(g,27,17,32,17,1); eye(g,16,19); eye(g,29,19)
        rect(g,21,28,27,28,1); px(g,28,27,1)
    nose(g); line(g,30,15,28,23,2)
    return outline(g,None,1)
def ren():
    g=blank(); uniform(g,False); face_base(g,wide=10,jaw=31)
    # short natural hair and a signals headset
    ell(g,24,12,13,9,1,lambda x,y:y<17)
    for x,y in ((16,8),(20,6),(25,6),(30,8),(33,11),(14,11)): px(g,x,y,2)
    rect(g,10,17,12,25,2); rect(g,36,17,38,25,2); line(g,12,25,18,28,2)
    line(g,16,18,21,18,1); line(g,27,18,32,18,1); eye(g,16,20); eye(g,29,20)
    nose(g); rect(g,21,28,27,28,1)
    # tired eyes
    line(g,16,23,19,23,2); line(g,29,23,32,23,2)
    return outline(g,None,1)
PORTRAITS=[('hale',hale('smile'),'smile'),('hale',hale('serious'),'serious'),('voss',voss('calm'),'calm'),('voss',voss('hard'),'hard'),('mara',mara('calm'),'calm'),('mara',mara('hard'),'hard'),('ren',ren(),'calm'),('log',blank(),'none')]
PAL={'hale':(0x07,0x27,0x37),'voss':(0x0C,0x10,0x37),'mara':(0x06,0x16,0x36),'ren':(0x08,0x17,0x27),'log':(0x0F,0x0F,0x0F)}  # dark, mid, light as NES colours
NESRGB={0x0F:(0,0,0),0x07:(90,44,0),0x27:(232,144,80),0x37:(255,214,170),0x0C:(0,60,90),0x10:(170,170,170),0x06:(120,20,10),0x16:(200,60,30),0x36:(255,190,170),0x01:(10,30,110),0x17:(170,90,20)}
if __name__=='__main__':
    from PIL import Image
    im=Image.new('RGB',(60*7+8,48+4),(40,40,40))
    for k,(n,g,e) in enumerate(PORTRAITS[:7]):
        for y in range(H):
            for x in range(W):
                c=g[y][x]; rgb=(0,0,0) if c==0 else NESRGB[PAL[n][c-1]]
                im.putpixel((2+k*60+x,2+y),rgb)
    im.resize((im.width*5,im.height*5),0).save('/tmp/portraits.png')
