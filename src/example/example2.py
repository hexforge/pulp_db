import random
import pprint
import sys
import struct

from pb_tools import parse_pb
import pulp_db
from hkseomd import get_spec, build_msg_parsers, make_decoder, pb_msg_decoder

SPEC_PATH = 'hkseomd.ini'
DS_PATH = "example2_ds"

spec = get_spec(SPEC_PATH)
decoders = build_msg_parsers(spec)
decoder = make_decoder(decoders, spec)
def full_decoder(msg):
    return decoder(pb_msg_decoder(msg)["payload"])

IDX_DUMPERS = {"msgtype": lambda msg_type: struct.pack("H", msg_type)}
IDX_LOADERS = {"msgtype": lambda raw_msg_type: struct.unpack("H", raw_msg_type)[0]}

def parse_msg_select_what_to_store(msg):
    raw = msg['msg']
    decoded_msg = decoder(msg['payload'])
    msg_types = set()
    for x in range(1, decoded_msg[0]["MsgCount"]+1):
        msg_type = decoded_msg[x]["MsgType"]
        msg_types.add(msg_type)
    index = {}
    index['ip'] = msg['ip'] 
    index['port'] = msg['port']
    index['time'] = msg['time']
    index['msgtype'] = msg_types
    return raw, index

# Write playback to db
count = 0
with pulp_db.open(DS_PATH, "w", idx_dumpers=IDX_DUMPERS) as db:
    for msg in parse_pb(sys.argv[1]):

        count += 1
        if count%10000 == 0:
            print("Counted", count)

        raw, index = parse_msg_select_what_to_store(msg)

        #continue

        db.append(raw, index)

sys.exit(1)

import pdb
pdb.set_trace()

# Read and query db
with pulp_db.open(DS_PATH, 
                  mode="r", 
                  msg_loader=full_decoder, 
                  idx_dumpers=IDX_DUMPERS,
                  idx_loaders=IDX_LOADERS,) as db:

    #import pdb
    #pdb.set_trace()

    print("---First message---")
    print(db[0].msg)

    import pdb
    pdb.set_trace()

    print("---Last message---")
    print(db[-1].msg)

    print("---Number of messages---")
    print(len(db))

    print("----First 10 messages")
    for x in db[:10]:
        print(x.msg)
    
    print("---Indexed fields---")
    print(db.idx.fields())  # 

    print("---All knowns ips---")
    print([x for x in db.idx['ip']])

