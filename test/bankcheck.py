# Fails if code that runs while a story bank is mapped can reach flight-bank code.
import re,sys
import glob
s=open('wire.s').read()+''.join(open(f).read() for f in sorted(glob.glob('src/*.inc')))
L={}
for l in open(sys.argv[1]):
    m=l.split(); L[m[2].lstrip('.')]=int(m[1],16)
body=''.join(open(f).read() for f in ('src/story_engine.inc','src/xfer.inc','src/sound.inc','src/boot.inc'))
bad=set()
for m in re.finditer(r'\b(jsr|jmp|lda|sta|ldx|ldy|adc|cmp|inc|dec)\s+([A-Za-z_][A-Za-z_0-9]*)',body):
    n=m.group(2)
    if n in L and 0xA000<=L[n]<0xC000: bad.add((m.group(1),n))
# new_game runs after flight_reset maps the flight bank; the story tables load while
# the assets bank is mapped; the rest are local labels that share a name.
ok={'new_game','storyAttr','storyMisc','advance','done','top'}
left=sorted(x for x in bad if x[1] not in ok)
print('unsafe references from always-mapped code:',left)
sys.exit(1 if left else 0)
