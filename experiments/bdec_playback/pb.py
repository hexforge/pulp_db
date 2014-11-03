from bdec.data import Data
from bdec import DecodeError
from bdec.spec.xmlspec import load
from bdec.spec import load_specs
from bdec.output.instance import decode

from bdec.spec.references import References
references = References()
import sys
with open(sys.argv[1], 'r') as pb:
    data = pb.read()
    spec = load_specs(["playback_faster.xml"])[0]
    try:
        values = decode(spec, Data(data))
    except DecodeError, err:
        print 'Oh oh...', err
        raise
    for value in values:
        print(value)
        break

print("done")
