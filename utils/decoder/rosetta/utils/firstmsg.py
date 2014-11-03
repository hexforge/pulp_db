import sys
import gzip
#from nyse_tech import pbparse

#x = pbparse(sys.argv[1])

#import pdb
#pdb.set_trace()

with gzip.open(sys.argv[1], 'rb') as x:
  y = next(x)

fname = sys.argv[1]+'.rip'

with open(fname, 'wb') as z:
    print(fname)
    for _ in range(int(sys.argv[2])):
        z.write(y)


