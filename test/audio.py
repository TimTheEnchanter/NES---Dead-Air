# Finds effect onsets in captured audio and compares them with recorded game events.
import sys
import numpy as np
pcm=np.fromfile(sys.argv[1],dtype=np.int16).reshape(-1,2).mean(1); rate=48000
R=256*240+40;raw=np.fromfile(sys.argv[2],dtype=np.uint8);N=len(raw)//R
st=raw.reshape(N,R)[:,256*240:].astype(int)
print('audio %.1f s, peak %d of 32767, clipped samples %d'%(len(pcm)/rate,abs(pcm).max(),(abs(pcm)>=32700).sum()))
win=rate//60; e=np.array([np.sqrt(np.mean(pcm[i:i+win]**2)) for i in range(0,len(pcm)-win,win)])
print('frames with signal %d of %d, median level %.0f'%((e>30).sum(),len(e),np.median(e)))
hf=[]
for i in range(0,len(pcm)-win,win):
    b=pcm[i:i+win]; s=np.abs(np.fft.rfft(b*np.hanning(len(b)))); f=np.fft.rfftfreq(len(b),1/rate)
    hf.append((s[(f>500)].sum()))
hf=np.array(hf); on=np.nonzero((hf[1:]>hf[:-1]*3+800))[0]+1
print('high-frequency onsets at ticks',on[:30].tolist(),'...' if len(on)>30 else '')
def events(col):
    return [t for t in range(1,N) if st[t,col]!=st[t-1,col]]
print('enemy shots',events(33)[:12],'bolt hits',events(34)[:12],'score',events(22)[:12])
def dom(t,n=3):
    # dominant pitch above the engine hum, n vblanks from tick t
    i=int(t*rate/60.1); b=pcm[i:i+int(rate/60*n)]
    if len(b)<256: return 0
    s=np.abs(np.fft.rfft(b*np.hanning(len(b)),n=1<<15)); f=np.fft.rfftfreq(1<<15,1/rate); m=f>300
    return float(f[m][s[m].argmax()])
def near(t):
    c=on[np.argmin(abs(on-t))] if len(on) else -999
    return int(c-t)
for t in events(33)[:4]: print('enemy shot tick',t,'onset offset',near(t),'pitch start %.0f, later %.0f Hz'%(dom(t+near(t)),dom(t+near(t)+10)))
for t in events(22)[:3]:
    s=e[t+near(t):t+near(t)+70]; print('kill tick',t,'onset offset',near(t),'audible above engine for %.2f s'%((s>np.median(e)*1.5+50).sum()/60))
