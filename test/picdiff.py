#!/usr/bin/env python3
"""Compares two PICDUMP files from test/bench.mjs (one 1-bit picture per update).
Reports frames that differ, pixels that differ, and pixels with no set pixel
within N pixels in the other picture (default 1): those are real changes,
the rest are one-pixel shifts."""
import sys, numpy as np
from scipy.ndimage import binary_dilation
a=np.unpackbits(np.fromfile(sys.argv[1],dtype=np.uint8)).reshape(-1,240,256).astype(bool)
b=np.unpackbits(np.fromfile(sys.argv[2],dtype=np.uint8)).reshape(-1,240,256).astype(bool)
tol=int(sys.argv[3]) if len(sys.argv)>3 else 1
n=min(len(a),len(b)); a,b=a[:n],b[:n]
st=np.ones((2*tol+1,2*tol+1),bool)
frames=diff=far=0; worst=(0,-1)
for i in range(n):
    d=a[i]^b[i]
    if not d.any(): continue
    frames+=1; diff+=int(d.sum())
    fa=a[i]&~binary_dilation(b[i],st); fb=b[i]&~binary_dilation(a[i],st)
    f=int(fa.sum()+fb.sum()); far+=f
    if f>worst[0]: worst=(f,i)
print(f"{n} pictures: {frames} differ, {diff} pixels differ, {far} further than {tol}px from any pixel in the other (worst frame {worst[1]}: {worst[0]})")
