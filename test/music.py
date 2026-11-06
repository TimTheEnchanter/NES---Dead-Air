# Checks music pitch, looping, and silence by screen, from captured audio.
import numpy as np
pcm=np.fromfile('/tmp/fl.pcm',dtype=np.int16).reshape(-1,2).mean(1);rate=48000
R=256*240+40;raw=np.fromfile('/tmp/flow.bin',dtype=np.uint8);N=len(raw)//R
s=raw.reshape(N,R)[:,256*240:].astype(int)
ch=[(t,int(s[t,19])) for t in range(1,N) if s[t,19]!=s[t-1,19]];print('modes',ch)
tick=rate/60.1
def seg(a,b): return pcm[int(a*tick):int(b*tick)]
def low(a,b):
    x=seg(a,b); sp=np.abs(np.fft.rfft(x*np.hanning(len(x)),n=1<<16)); f=np.fft.rfftfreq(1<<16,1/rate)
    m=(f>30)&(f<400); return round(float(f[m][sp[m].argmax()]),1),round(float(np.sqrt(np.mean(x**2))))
print('title start: strongest pitch, level',low(5,115))
print('during pause in game-over sting',low(ch[1][0]+2,ch[1][0]+90))
print('after sting ends, level',low(ch[1][0]+95,ch[2][0]-1))
print('title again',low(ch[2][0]+10,ch[3][0]-5))
print('in play, low band',low(ch[3][0]+40,ch[4][0]-40))
