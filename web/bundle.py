#!/usr/bin/env python3
"""Bundles the jsnes ES modules into one classic script (npm is unavailable here)."""
import re,os,sys
root=os.path.expanduser('~/tools/jsnes/src')
seen={};order=[]
def load(p):
    p=os.path.normpath(p)
    if p in seen: return
    seen[p]=1
    s=open(p).read(); exp=[]
    def imp(m):
        names,rel=m.group(1).strip(),m.group(2)
        t=os.path.normpath(os.path.join(os.path.dirname(p),rel)); load(t); key=os.path.relpath(t,root)
        if names.startswith('{'):
            return f'const {names.replace(" as ",":")}=__req("{key}");'
        return f'const {names}=__req("{key}").default;'
    s=re.sub(r'^import\s+(.+?)\s+from\s+"([^"]+)";',imp,s,flags=re.M)
    def dcl(m):
        exp.append(f'exports.default={m.group(2)};'); return m.group(1)+' '+m.group(2)
    s=re.sub(r'^export default (class|function) (\w+)',dcl,s,flags=re.M)
    s=re.sub(r'^export default \{','exports.default = {',s,flags=re.M)
    s=re.sub(r'^export default (\w+);',lambda m:(exp.append(f'exports.default={m.group(1)};'),'')[1],s,flags=re.M)
    s=re.sub(r'^export (function|const|class) (\w+)',lambda m:(exp.append(f'exports.{m.group(2)}={m.group(2)};'),f'{m.group(1)} {m.group(2)}')[1],s,flags=re.M)
    left=re.findall(r'^(?:export|import)\b.*',s,flags=re.M)
    if left: sys.exit(f'{p}: unhandled {left[:3]}')
    order.append((os.path.relpath(p,root),s+'\n'+'\n'.join(exp)))
load(os.path.join(root,'nes.js'))
out=['(function(){const __m={};const __req=k=>__m[k];']
for k,s in order:
    out.append(f'__m["{k}"]=(function(){{const exports={{}};\n{s}\nreturn exports;}})();')
out.append('window.JSNES={NES:__m["nes.js"].default};})();')
open('jsnes.bundle.js','w').write('\n'.join(out).replace('</script','<\\/script'))
print(len(order),'modules',sum(len(x) for x in out))
