import itertools
import random
from collections import namedtuple
from nose.tools import raises

import pypath_hack
import pulp_db
from pb_tools import get_decoder, parse_pb

MSG_TYPES = ['A'.encode('ascii'), 'B'.encode('ascii'), 'C'.encode('ascii')]
REGULAR_MSG_TYPES = itertools.cycle(MSG_TYPES)

element = namedtuple('element', ['raw', 'index'])

#---
# Create a data set
#---
DATA_SET = None
def create_db(db_name, playback, spec, index_what):
    global DATA_SET
    DATA_SET = []
    with pulp_db.open(db_name, "w") as db:
        for msg in parse_pb(playback):
            index = {}
            index['ip'] = msg['ip'] 
            index['port'] = msg['port']
            index['time'] = msg['time']
            index['type'] =  next(REGULAR_MSG_TYPES)

            #size = msg['header_size'] + msg['payload_size']
            raw = msg['msg']

            DATA_SET.append(element(raw, index))
            db.append(raw, index)

DB = None
all_ips = None
all_ports = None
all_times = None
all_types = None
def setup_module():
    create_db("data/pulp/end_to_end/example_ds", 
              "data/pulp/end_to_end/example.pb", 
              "data/pulp/end_to_end/example.spec", 
              None)

    global DB, all_ips, all_ports, all_times, all_types
    DB =  pulp_db.open("data/pulp/end_to_end/example_ds", "r")
    DB.__enter__()

    all_ips = {x.index['ip'] for x in DATA_SET}
    all_ports = {x.index['port'] for x in DATA_SET}
    all_times = {x.index['time'] for x in DATA_SET}
    all_types = {x.index['type'] for x in DATA_SET}

def teardown_module():
    global DB
    assert DB is not None
    DB.__exit__(None, None, None)
    DB = None

#---
# Test
#---
def test_get_first_message():
    print(DB[0].msg == DATA_SET[0].raw)

@raises(IndexError)
def check_boundary_lookup(i):
    DB[i]

def should_be_fine(i):
    DB[i]

def test_index_out_of_bounds():
    for x in range(3):
        yield check_boundary_lookup, len(DATA_SET) + x
    
    for x in range(-1, -10, -1):
        yield should_be_fine,  x

    yield should_be_fine, -1000

    for x in range(-(len(DATA_SET)+1), -(len(DATA_SET)+10), -1):
        yield check_boundary_lookup,  x


def test_get_first_ten_messages():
    actual_first_10 = [x.msg for x in DB[:10]]
    expected_first_10 = [x.raw for x in DATA_SET[:10]]
    assert (actual_first_10 == expected_first_10)



def test_all_messages():
    for actual, expected in itertools.zip_longest(DB, DATA_SET):
        print(actual.msg, expected.raw, actual.msg == expected.raw)
        assert(actual.msg == expected.raw)

# Doesn't support negative at the moment
#def test_get_last_ten_messages():
#    for x in db[-10:]:
#        print(x.msg)

def test_num_msgs():
    print(len(DB) == len(DATA_SET))

def test_num_of_keys():
    assert (len(DB.idx['ip']) == len(all_ips))
    assert (len(DB.idx['port']) == len(all_ports))
    assert (len(DB.idx['time']) == len(all_times))
    assert (len(DB.idx['type']) == len(all_types))

def check_num_refs(field, thing):
    assert (len(DB.idx[field][thing]) == len([x for x in DATA_SET if x.index[field]==thing]))

def test_num_of_refs():
    for ip in all_ips:
        yield check_num_refs, "ip", ip
    for port in all_ports:
        yield check_num_refs, "port", port
    for time in all_times:
        yield check_num_refs, "time", time
    for typ in all_types:
        yield check_num_refs, "type", typ

def test_indexed_fields():
    assert {x for x in DB.idx.fields()} == {"ip", "port", "time", "type"}

def test_can_get_random_ip():
    assert random.choice(list(DB.idx['ip'].keys())) in all_ips

def check_single_query(field, thing):
    actual = [(x.id, x.msg) for x in DB.idx[field][thing]]
    expected = [(i, x.raw) for i, x in enumerate(DATA_SET) if x.index[field]==thing]
    assert actual == expected

def test_all_single_queries():
    for ip in all_ips:
        yield check_single_query, "ip", ip
    for port in all_ports:
        yield check_single_query, "port", port
    for time in all_times:
        yield check_single_query, "time", time
    for typ in all_types:
        yield check_single_query, "type", typ

@raises(KeyError)
def test_query_not_there():
    DB.idx["NOT THERE"][thing]

todo = """
    print("--All messages for ip=233.37.54.12 and time=1325626218.359585---")
    for x in db.idx['ip'][b'233.37.54.12'] & db.idx['time'][b'1325626218.359585']:
        print(x.msg)

    print("--All messages for ip=233.37.54.12 when time is not time=1325626218.359585---")
    for x in db.idx['ip'][b'233.37.54.12'] - db.idx['time'][b'1325626218.359585']:
        print(x.msg)

    print("--All messages for ip=233.37.54.13 or b'233.37.54.53'---")
    ip_query1 = db.idx['ip'][b'233.37.54.13']
    ip_query2 = db.idx['ip'][b'233.37.54.53']
    for x in ip_query1|ip_query2:
        print(x.msg)
    
    print("---All messages where the last digit is even ---")
    def even_ip(ip):
        print("in function", ip, ip[-1], ip[-1]==b'3'[0])

        return ip[-1]==b'3'
        #return int(ip[-1])%2==0

    for x in db.idx(ip=even_ip):
        print(1111)
        print(x.msg)
"""
