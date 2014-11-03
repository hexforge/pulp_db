import sys

from bdec.data import Data
from bdec import DecodeError
from bdec.spec.xmlspec import load
from bdec.spec import load_specs
from bdec.output.instance import decode

from bdec.spec.references import References
references = References()

def get_pb_line(fpath):
    with open(fpath) as f:
        for line in f:
            yield line

count = 0 
with open('playback.xml', 'r') as xml_f:
    spec = load_specs(["playback.xml"])[0]
    for line in get_pb_line(sys.argv[1]):
        try:
            values = decode(spec, Data(line))
        except DecodeError, err:
            print 'Oh oh...', err
            raise
        count += 0
        print(values)
        #import pdb
        #pdb.set_trace()
        #print(2)

print("done", count)
