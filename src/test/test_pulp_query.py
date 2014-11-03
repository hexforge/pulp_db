from collections import namedtuple, ChainMap
import random

from nose import with_setup
from nose.tools import raises

import pulp_db
from pb_tools import get_decoder, parse_pb


element = namedtuple('element', ['raw', 'index'])

# Some test data to populate db with
str_1 = b"First spy \0message"
msg_1 = element(raw=str_1, 
                index={'sym': b'spy', 
                       'ip':b'127.0.0.1', 
                       'msg_type': b'A',
                       'has_null': b'True',
                       }
                )

str_2 = b"First blah message"
msg_2 = element(raw=str_2, 
                index={'sym': b'blah', 
                       'ip': b'127.0.0.2', 
                       'msg_type': b'A',
                       }
               )


str_3 = b"Second spy message"
msg_3 = element(raw=str_3, 
                index={'sym': b'spy', 
                       'ip': b'127.0.0.1', 
                       'msg_type': b'B'
                       }
               )


str_4 = b"Third spy message"
msg_4 = element(raw=str_4, 
                index={'sym': b'spy', 
                       'ip': b'127.0.0.1', 
                       'msg_type': b'C',
                       'thingy': b'12121'
                       }
               )


str_5 = b"First erm \0message"
msg_5 = element(raw=str_5, 
                index={'sym': b'erm', 
                       'ip': b'127.0.0.2', 
                       'msg_type': b'A',
                       'has_null': b'True'
                       }
               )

str_6 = b"Second blah message"
msg_6 = element(raw=str_6, 
                index={'sym': b'blah', 
                       'ip': b'127.0.0.1', 
                       'msg_type': b'C'
                       }
               )

DATA = [msg_1, msg_2, msg_3, msg_4, msg_5, msg_6]
DB = None

TONS_OF_DATA = []
DB_TONS = None
def generate_ton_data():
    for x in range(1000):
        payload = ("msg<" + str(x) + ">").encode('ascii')
        if x%2 == 0:
            sym = 'spy'.encode('ascii')
        else:
            sym = 'foo'.encode('ascii')

        msg = element(raw=payload, index={'sym': sym})
        TONS_OF_DATA.append(msg)
generate_ton_data()

#---
# DB setup
#---
def create_db(db_name, data):
    with pulp_db.open(db_name, "w") as db:
        for e in data:
            db.append(e.raw, e.index) 

def setup_module():
    db_name = "data/pulp/query/example_ds"
    create_db(db_name, DATA)
    global DB
    assert DB is None
    DB = read_pulp_setup(db_name)
    assert DB is not None

    db_name2 = "data/pulp/query/example_tons"
    create_db(db_name2, TONS_OF_DATA)
    global DB_TONS
    assert DB_TONS is None
    DB_TONS = read_pulp_setup(db_name2)
    assert DB_TONS is not None

def teardown_module():
    global DB
    DB.__exit__(None, None, None)
    DB = None

    global DB_TONS
    DB_TONS.__exit__(None, None, None)
    DB_TONS = None

def read_pulp_setup(db_name):
    x = pulp_db.open(db_name, "r")
    x.__enter__()
    return x

#---
# Tests
#---
def test_get_first_message():
    print(DB[0])
    print(DB[0].id, 0, DB[0].id == 0)
    assert DB[0].id == 0
    print(DB[0].msg, DATA[0].raw, DB[0].msg == DATA[0].raw)
    assert DB[0].msg == DATA[0].raw

@raises(IndexError)
def test_boundary_geti():
    print(DB[len(DATA)+10])
    print("hgello")

def test_num_messages_in_db():
    print("Number messages in data=", len(DATA))
    print("Number of messages in db=", len(DB))
    assert len(DB) == len(DATA)

def test_2to4_msgs():
    print("type", type(DB))
    actual_msgs = [x.msg for x in DB[2:4]]
    expected_msgs = [x.raw for x in DATA[2:4]]
    print("actual={}".format(actual_msgs))
    print("expected={}".format(expected_msgs))
    assert actual_msgs == expected_msgs

def test_list_indexed_fields():
    expected_keys = sorted(ChainMap(*[x.index for x in DATA]))
    actual_keys_1 = sorted(DB.idx.keys())
    actual_keys_2 = sorted(DB.idx.fields())
    print("expected={}".format(expected_keys))
    print("actual_1={}".format(actual_keys_1))
    print("actual_2={}".format(actual_keys_2))
    assert expected_keys == actual_keys_1 == actual_keys_2

@raises(KeyError)
def test_idx_not_exist():
    DB.idx['SomeNonThing']

def test_get_first_sym():
    sym = list(DB.idx['sym'].keys())[0]
    assert sym in set(x.index['sym'] for x in DATA)

def test_num_syms():
    actual_num_syms = len(DB.idx['sym'])
    expected_num_syms = len({x.index['sym'] for x in DATA})
    print("actual_num_syms  ", actual_num_syms)
    print("expected_num_syms", expected_num_syms)
    assert actual_num_syms == expected_num_syms

def test_random_choice():
    random_sym = random.choice(list(DB.idx['sym'].keys()))
    print(random_sym)
    assert random_sym in set(x.index['sym'] for x in DATA)

def test_key_for_field():
    actual_all_syms = sorted(DB.idx['sym'].keys())
    actual_all_syms2 = sorted(sym for sym in DB.idx['sym'].keys())
    expected_all_syms = sorted(set(x.index['sym'] for x in DATA))
    print("expected={}".format(actual_all_syms))
    print("actual_via_slice1={}".format(actual_all_syms2))
    print("actual_via_iter={}".format(expected_all_syms))
    assert actual_all_syms == actual_all_syms2 == expected_all_syms

def test_every_other_sym():
    some_syms = list(DB.idx['sym'].keys())[::2]
    assert len(some_syms) == 2

def test_most_common_sym():
    freq_sym = [(len(DB.idx['sym'][s]), s) for s in DB.idx['sym']]
    freq_sym.sort(reverse=True)
    actual_top_2sym = [s for length, s in freq_sym[:2]]
    expected_top2_sym = [b'spy', b'blah'] 
    print("expected = ", expected_top2_sym)
    print("actual =", actual_top_2sym)
    assert actual_top_2sym == expected_top2_sym

def test_msg_for_a_sym():
    actual_msgs = [x.msg for x in DB.idx['sym'][b'spy']]
    expected_msgs = [x.raw for x in DATA if x.index['sym']==b'spy']
    print("actual_msgs  ", actual_msgs)
    print("expected_msgs", expected_msgs)
    assert actual_msgs == expected_msgs

def test_all_msg_types_A():
    expected_data = [x.raw for x in DATA if x.index['msg_type']==b'A']
    s1_query = DB.idx['msg_type'][b'A']
    actual_data = [x.msg for x in s1_query]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data 

def test_num_spy_msgs():
    actual_num_spy_msgs = len(DB.idx['sym'][b'spy'])
    expected_num_spy_msgs = len([msg for msg in DATA if msg.index['sym']==b'spy'])
    print("actual_num_spy_msgs  ", actual_num_spy_msgs)
    print("expected_num_spy_msgs", expected_num_spy_msgs)
    assert actual_num_spy_msgs == expected_num_spy_msgs

def test_keyquery():
    def get_books(symbol):
        #print("wddwdwd", symbol in {b'erm', b'blah'}, symbol)
        return symbol in {'erm'.encode('ascii'), 'blah'.encode('ascii')}

    expected_data = [x.raw for x in DATA if x.index['sym'] in {'erm'.encode('ascii'), 'blah'.encode('ascii')}]

    virt_q = DB.idx['sym'][b'erm'] | DB.idx['sym']['blah'.encode('ascii')]
    actual_sym_call_join = [x.msg for x in virt_q]
    actual_idx_call = [x.msg for x in DB.idx(sym=get_books)]
    actual_sym_call = [x.msg for x in DB.idx['sym'](get_books)]

    print("expected_data       :", expected_data)
    print("actual_idx_call     :", actual_idx_call)
    print("actual_sym_call     :", actual_sym_call)
    print("actual_sym_call_join:", actual_sym_call_join)
    assert expected_data == actual_idx_call == actual_sym_call == actual_sym_call_join

def test_or():
    expected_data = [x.raw for x in DATA 
                     if x.index['msg_type']=='A'.encode('ascii') 
                     or x.index['msg_type']=='C'.encode('ascii')]
    s1_query = DB.idx['msg_type']['A'.encode('ascii')]
    s2_query = DB.idx['msg_type']['C'.encode('ascii')]
    actual_data = [x.msg for x in (s1_query | s2_query)]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_and():
    expected_data = [x.raw for x in DATA 
                     if x.index['msg_type']=='A'.encode('ascii') 
                     and x.index.get('has_null')==b'True']
    s1_query = DB.idx['msg_type']['A'.encode('ascii') ]
    s2_query = DB.idx['has_null']['True'.encode('ascii')]

    print("msgtype_A_query", s1_query)
    print("has_null_True_query", s2_query)

    actual_data = [x.msg for x in (s1_query & s2_query)]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_sub():
    expected_data = [x.raw for x in DATA 
                     if x.index['msg_type']=='A'.encode('ascii') 
                     and x.index.get('has_null')!='True'.encode('ascii')]

    s1_query = DB.idx['msg_type']['A'.encode('ascii')]
    s2_query = DB.idx['has_null']['True'.encode('ascii')]
    actual_data = [x.msg for x in (s1_query - s2_query)]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_xor():
    expected_data = [x.raw for x in DATA 
                     if (x.index['msg_type']=='A'.encode('ascii') or x.index.get('has_null')=='True'.encode('ascii'))
                     and not (x.index['msg_type']=='A'.encode('ascii') and x.index.get('has_null')=='True'.encode('ascii'))]
    s1_query = DB.idx['msg_type']['A'.encode('ascii')]
    s2_query = DB.idx['has_null']['True'.encode('ascii')]
    actual_data = [x.msg for x in (s1_query ^ s2_query)]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_vquery_with_normal_query():
    expected_data = [x.raw for x in DATA 
                     if x.index['msg_type']=='A'.encode('ascii') 
                     or x.index['msg_type']=='B'.encode('ascii') 
                     or x.index['msg_type']=='C'.encode('ascii')]
    

    msgtype_A_query = DB.idx['msg_type']['A'.encode('ascii')]
    msgtype_B_query = DB.idx['msg_type']['B'.encode('ascii')]
    msgtype_C_query = DB.idx['msg_type']['C'.encode('ascii')]
    vquery = (msgtype_A_query | msgtype_B_query)
    combo_query = (vquery | msgtype_C_query)

    print("msgtype_A_query", msgtype_A_query)
    print("msgtype_B_query", msgtype_B_query)
    print("msgtype_C_query", msgtype_C_query)
    print("msgtype_vquery", vquery)
    print("msgtype_combo_query", combo_query)
    
    actual_data = [x.msg for x in combo_query]

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_iterate_over_all_syms():
    expected_data = [x.raw for x in DATA if 'sym' in x.index]
    actual_data_any = [x.msg for x in DB.idx['sym'].any()]
    actual_data_call = [x.msg for x in DB.idx(sym=lambda sym: True)]
    print("expected_data    :", expected_data)
    print("actual_data_any  :", actual_data_any)
    print("actual_data_call  :", actual_data_call)
    assert expected_data == actual_data_any == actual_data_call

def test_get_slice_msgs():
    expected_data = [x.raw for x in DATA[1:3]]
    actual_data = [x.msg for x in DB[1:3]]    # This maybe should be a streamquery so it can be joined.
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data 

def test_stream_query():
    expected_data = [x.raw for x in DATA if '\0message'.encode('ascii') in x.raw]
    def my_query(msg):
        return b'\0message' in msg
    
    actual_data = [x.msg for x in DB.stream(my_query)]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_stream_null_query():
    expected_data = [x.raw for x in DATA if 'wjjwdkjn'.encode('ascii') in x.raw]
    def my_query(msg):
        return 'wjjwdkjn'.encode('ascii') in msg
    
    actual_data = [x.msg for x in DB.stream(my_query)]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_slice_db_combine():
    expected_data = [x.raw for x in DATA[:3] if x.index['sym']=='spy'.encode('ascii')]
    actual_data_1 = [x.msg for x in DB[:3]&DB.idx['sym']['spy'.encode('ascii')]]
    actual_data_2 = [x.msg for x in DB[:3]&DB.idx(sym='spy'.encode('ascii'))]
    print("expected_data :", expected_data)
    print("actual_data_1 :", actual_data_1)
    print("actual_data_2 :", actual_data_2)
    assert expected_data == actual_data_1 == actual_data_2

def test_stream_key_vquery():
    expected_data = [x.raw for x in DATA if '\0message'.encode('ascii') in x.raw and x.index['sym']=='spy'.encode('ascii')]
    def my_query(msg):
        return '\0message'.encode('ascii') in msg
    actual_data = [x.msg for x in DB.stream(my_query)&DB.idx['sym']['spy'.encode('ascii')]]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_stream_vquery_combo():
    expected_data = [x.raw for x in DATA if '\0message'.encode('ascii') in x.raw and x.index['sym']=='spy'.encode('ascii')]
    def my_query(msg):
        return '\0message'.encode('ascii') in msg
    actual_data = [x.msg for x in DB.stream(my_query)&DB.idx['sym']['spy'.encode('ascii')]&DB.idx['sym']['spy'.encode('ascii')]&DB.stream(my_query)]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data


def test_2stream_vquery():
    print([x.raw for x in DATA])
    expected_data = [x.raw for x in DATA if '\0message'.encode('ascii') in x.raw and 'spy'.encode('ascii') in x.raw]
    def my_query(msg):
        return '\0message'.encode('ascii') in msg
    def my_query2(msg):
        print("my_query2 called", 'spy'.encode('ascii') in msg, msg)
        return 'spy'.encode('ascii') in msg

    actual_data = [x.msg for x in DB.stream(my_query2)&DB.stream(my_query)]

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data


def test_double_query():
    print([x.raw for x in DATA])
    expected_data = [x.raw for x in DATA if 'ird'.encode('ascii') in x.raw and 'spy'.encode('ascii') in x.raw]
    def my_query(msg):
        return 'spy'.encode('ascii') in msg
    def my_query2(msg):
        return 'ird'.encode('ascii') in msg

    actual_data = [x.msg for x in DB.stream(my_query2)(my_query)]

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_query_slice():
    print([x.raw for x in DATA])
    expected_data = [x.raw for x in DATA if 'blah'.encode('ascii') in x.raw][0:2]
    def my_query(msg):
        return 'blah'.encode('ascii') in msg
    actual_data = [x.msg for x in DB.stream(my_query)[0:2]]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_query_ands():
    print([x.raw for x in DATA])
    expected_data = [x.raw for x in DATA if 'blah'.encode('ascii') in x.raw and 'ird'.encode('ascii') in x.raw]
    def my_query(msg):
        return 'blah'.encode('ascii') in msg
    def my_query2(msg):
        return 'ird'.encode('ascii') in msg
    actual_data = [x.msg for x in DB.stream(my_query) & DB.stream(my_query2)]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_query_ands_with_slice():
    print([x.raw for x in DATA])
    expected_data = [x.raw for x in DATA if 'blah'.encode('ascii') in x.raw][:1]
    def my_query(msg):
        return 'blah'.encode('ascii') in msg
    def my_query2(msg):
        return 'ird'.encode('ascii') in msg
    actual_data = [x.msg for x in DB.stream(my_query) & DB.stream(my_query)[:1]]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data


def test_query_ands_with_slice2():
    print([x.raw for x in DATA])
    expected_data = [x.raw for x in DATA if 'blah'.encode('ascii') in x.raw][:1]
    def my_query(msg):
        return 'blah'.encode('ascii') in msg
    actual_data = [x.msg for x in (DB.stream(my_query) & DB.stream(my_query))[:1]]
    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data


def test_forwards_backwards():
    print([x.raw for x in DATA])
    expected_data = [x.raw for x in DATA]
    def my_query(msg):
        return 'blah'.encode('ascii') in msg

    x = DB.stream
    results = []
    while True:
        try:
            msg = x.next()
        except StopIteration:
            break
        print(msg)
        results.append(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", results)
    assert expected_data == results

    while True:
        try:
            msg = x.prev()
        except StopIteration:
            break
        results.remove(msg.msg)

    assert results == []

def test_forwards_backwards_with_query():
    expected_data = [x.raw for x in DATA if 'blah'.encode('ascii') in x.raw]
    print(expected_data)
    def my_query(msg):
        return 'blah'.encode('ascii') in msg

    x = DB.stream(my_query)
    results = []
    while True:
        try:
            msg = x.next()
        except StopIteration:
            break
        print(msg)
        results.append(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", results)
    assert expected_data == results

    expected_data = []
    while True:
        try:
            msg = x.prev()
            print(msg)
        except StopIteration:
            print("Stopping")
            break
        print("jwiojdij", msg)
        results.remove(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", results)
    assert results == expected_data

def test_forwards_backwards_with_query_slice():
    expected_data = [x.raw for x in DATA if 'blah'.encode('ascii') in x.raw][1:2]
    print(expected_data)
    def my_query(msg):
        return 'blah'.encode('ascii') in msg

    x = DB.stream(my_query)[1:2]
    results = []
    while True:
        try:
            msg = x.next()
            print("next=", msg)
        except StopIteration:
            break
        print(msg)
        results.append(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", results)
    assert expected_data == results

    expected_data = []
    while True:
        try:
            msg = x.prev()
            print("prev=", msg)
        except StopIteration:
            print("Stopping")
            break
        results.remove(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", results)
    assert results == expected_data


def test_backwards_forwards__with_query_slice():
    expected_data = [x.raw for x in DATA if 'blah'.encode('ascii') in x.raw][1:2]
    print(expected_data)
    def my_query(msg):
        return 'blah'.encode('ascii') in msg

    x = DB.stream(my_query)[1:2]
    results = []
    while True:
        try:
            msg = x.prev()
        except StopIteration:
            break
        print(msg)
        results.append(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", results)
    assert set(expected_data) == set(results)

    expected_data = []
    while True:
        try:
            msg = x.next()
            print(msg)
        except StopIteration:
            print("Stopping")
            break
        results.remove(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", results)
    assert results == expected_data

def test_forwards_backwards_with_vquery():
    expected_data = [x.raw for x in DATA if x.index['ip']=='127.0.0.1'.encode('ascii')  and x.index['msg_type']=='C'.encode('ascii')]
    print(expected_data)

    x = (DB.idx['ip']['127.0.0.1'.encode('ascii')] & DB.idx['msg_type']['C'.encode('ascii')])
    
    actual_data = []
    while True:
        try:
            msg = x.next()
            print(msg)
        except StopIteration:
            print("Stopping")
            break
        actual_data.append(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)

    #import pdb
    #pdb.set_trace()

    assert expected_data == actual_data

    expected_data = []
    while True:
        try:
            msg = x.prev()
        except StopIteration:
            break
        print(msg)
        #import pdb
        #pdb.set_trace()
        actual_data.remove(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_backwards_forwards_with_vquery():
    expected_data = [x.raw for x in DATA if x.index['ip']=='127.0.0.1'.encode('ascii') and x.index['msg_type']=='C'.encode('ascii')]
    print(expected_data)

    x = (DB.idx['ip']['127.0.0.1'.encode('ascii')] & DB.idx['msg_type']['C'.encode('ascii')])
    
    actual_data = []
    while True:
        try:
            msg = x.prev()
            print(msg)
        except StopIteration:
            print("Stopping")
            break
        actual_data.append(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert set(expected_data) == set(actual_data)

    expected_data = []
    while True:
        try:
            msg = x.next()
        except StopIteration:
            break
        print(msg)
        actual_data.remove(msg.msg)

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert expected_data == actual_data

def test_sym_query_slice():
    expected_data = [x.raw for x in TONS_OF_DATA if 'spy'.encode('ascii') in x.index['sym']][:100]
    
    query = DB_TONS.idx(sym='spy'.encode('ascii'))[:100]     # Get first 100 spy messages
    actual_data = [x.msg for x in query]

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert actual_data == expected_data

@raises(KeyError)
def test_not_there_field_query():
    query = DB_TONS.idx(not_there='spy'.encode('ascii'))[:100]    # Get first 100 spy messages


def test_sym_vquery_slice_combo():
    expected_data = [x.raw for x in TONS_OF_DATA if 'spy'.encode('ascii') in x.index['sym']][:100]
    
    query = DB_TONS.idx(sym='spy'.encode('ascii')) & DB_TONS.idx(sym='spy'.encode('ascii'))[:100]     # Get first 100 spy messages
    actual_data = [x.msg for x in query]

    print("expected_data :", expected_data)
    print("actual_data   :", actual_data)
    assert actual_data == expected_data

######################################################################

def run_tsts():
    for name, thing in dict(globals()).items():
        
        if name not in {'test_forwards_backwards_with_vquery'}:
            continue
        if callable(thing) and name.startswith('test'):
            print("Calling test=", name)
            thing()


if __name__ == '__main__':
    setup_module()
    run_tsts()
    teardown_module()
    


    

