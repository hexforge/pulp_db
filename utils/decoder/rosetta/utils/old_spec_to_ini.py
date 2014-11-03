import sys
import os
import ConfigParser
import string
from collections import OrderedDict

x = os.path.dirname(sys.argv[1])
sys.path.append(x)

import spec_lib
from spec_lib.bin_ascii import ascii_bin
from spec_lib.generic_feedhandler import generic_feedhandler

files = [y.split('.', 1)[0] for y in os.listdir(x) if os.path.isfile(os.path.join(sys.argv[1], y)) and not y.startswith('#') and y.endswith('.py')]

def transform_msg(transformed_msgs, msg_type, vals):
    transformed_msgs[str(msg_type)] = []

    for val in vals:
        if isinstance(val, str):
            fld, _, val = val.partition('=')

            y = set(fld[0]) - set(list(string.ascii_letters) + list(string.digits) + ['_', '*'])
            if y:
                if fld[0] in {'&', '%', '?'}:
                    fld = fld[1:]
            if fld[0] == '*':
                val = None

            fld = fld.replace(' ', '_')
            transformed_msgs[str(msg_type)].append((fld, val))

        if isinstance(val, list):
            transform_msg(transformed_msgs, str(msg_type)+'.sub', val)

for f in files:
    m = __import__(f)
    print(m.feed)
    ofname = os.path.join(sys.argv[2], f+'.ini')
    print(ofname)

    #spaces to underscores

    transformed_msgs = OrderedDict()

    for msg_type, vals in m.feed.msg_dict.items():
        print(msg_type, vals)
        transform_msg(transformed_msgs, msg_type, vals)

    with open(ofname, 'w') as o:
        x = ConfigParser.RawConfigParser(allow_no_value=True)

        #x.add_section("DEFAULT")
        x.set("DEFAULT", "_endian", m.feed.endian)
        for msg, spec in transformed_msgs.items(): 

            x.add_section(msg)
            for fld, val in spec:
                print(o, msg, fld, val)
                x.set(msg, fld, val)
        x.write(o)
        