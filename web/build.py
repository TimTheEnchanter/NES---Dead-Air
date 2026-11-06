#!/usr/bin/env python3
import base64
s=open('page.src.html').read()
s=s.replace('/*JSNES*/',open('jsnes.bundle.js').read()).replace('/*ROM*/',base64.b64encode(open('../dead-air.nes','rb').read()).decode())
open('dead-air.html','w').write(s)
print(len(s))
