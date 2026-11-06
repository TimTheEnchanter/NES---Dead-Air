import json,numpy as np,collections
r=np.array(json.load(open('/tmp/fit.json')),float)[60:]
g=collections.defaultdict(list)
for row in r: g[tuple(int(v) for v in row[1:])].append(row[0])
for k,v in sorted(g.items(),key=lambda kv:-len(kv[1])): print(k,len(v),min(v),max(v))
