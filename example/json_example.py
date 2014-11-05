
from collections import namedtuple, ChainMap
import random
import json
import struct

import pulp_db


#---
# DATA
#---
msg_1 = {"name": "batman",
         "age": 25,
         "string": "I am batman",
         }

msg_2 = {"name": "batman",
         "age": 5,
         "string": "I am also batman",
         }

msg_3 = {"name": "batman",
         "string": "I am also batman?",
         }

msg_4 = {"name": "robin",
         "age": 25,
         "string": "I am not batman",
         }

msg_5 = {"name": "superman",
         "age": 2000,
         "string": "I am superman",
         }

DATA = [msg_1, msg_2, msg_3, msg_4, msg_5]


#---
# Utils
#---
# Need a few serialisers.

def dump_json(msg):
    return json.dumps(msg).encode()

def load_json(raw_msg):
    return json.loads(raw_msg.decode())

def dump_int(i):
    return struct.pack("I", i)

def load_int(raw_i):
    return struct.unpack("I", raw_i)[0]

def dump_str_to_bytes(string):
    return string.encode()

def load_str_to_bytes(raw):
    return raw.decode() 

MSG_DUMPER = dump_json
MSG_LOADER = load_json

IDX_DUMPERS = {"name": dump_str_to_bytes,
               "age": dump_int,
              }

IDX_LOADERS = {"name": load_str_to_bytes,
               "age": load_int,
              }




#---
# Write the DB
#---
db_name = "data/pulp/query/json_ds"
with pulp_db.open(db_name, "w", msg_dumper=MSG_DUMPER, idx_dumpers=IDX_DUMPERS) as db:
    for msg in DATA:
        
        # What to index
        index = {"name": msg["name"]}
        if "age" in msg:
            index["age"] =  msg["age"]

        # Put the data into the database.   
        # db.append(b'the_bin_json_data', {"name": "foo", "age": 5})
        db.append(msg, index) 






#--- 
# Read the DB
#---
with pulp_db.open(db_name, 
                    mode="r",
                    msg_dumper=MSG_DUMPER,
                    msg_loader=MSG_LOADER,
                    idx_dumpers=IDX_DUMPERS,
                    idx_loaders=IDX_LOADERS,
                    ) as DB:


    

    num_messages = len(DB)

    first_msg = DB[0]      #<-- This is a message object.
    first_msg.msg  #<--- This is the data_blob
    first_msg.id   #<--- This is the position of the data_blob. 0 = first message of the day
    
    last_msg = DB[-1]      #<-- This is a message object.
    last_msg_json = last_msg.msg  #<--- This is the data_blob
    last_msg_id = last_msg.id   #<--- This is the position of the data_blob. 0 = first message of the day
    

    first_two_messages = DB[:2]
    assert [x.msg for x in first_two_messages] == DATA[:2]

    last_two_messages = DB[-2:]
    assert [x.msg for x in last_two_messages] == DATA[-2:]

    actual_all_msgs = [x.msg for x in DB] 
    assert actual_all_msgs == DATA

    
    # Forward Iterator  (The same as above next == __next__)
    actual_msgs = []
    db_stream = DB.stream
    while True:
        try:
            msg = db_stream.next()
        except StopIteration:
            break
        else:
            actual_msgs.append(msg.msg)

    expected_messages = DATA
    assert actual_msgs == expected_messages


    #Backward Iterator ----------------------------
    actual_msgs = []
    db_stream = DB.stream
    while True:
        try:
            msg = db_stream.prev()
        except StopIteration:
            break
        else:
            actual_msgs.append(msg.msg)

    expected_messages = DATA[::-1]
    assert actual_msgs == expected_messages



    #test_jump_then_forwards_backwards----------------------------

    db_stream = DB.stream
    actual_msg_3 = db_stream[3]
    assert actual_msg_3.msg == DATA[3]

    expected_msg_4 = db_stream.next()
    assert expected_msg_4.msg == DATA[4]

    expected_msg_3 = db_stream.prev()
    assert expected_msg_3.msg == DATA[3]

    expected_msg_2 = db_stream.prev()
    assert expected_msg_2.msg == DATA[2]


    #test_all_indexed_fields----------------------------
    indexed_fields = DB.idx.fields()
    assert set(indexed_fields) == {"name", "age"}




    #test_all_names----------------------------
    actual_names = set(DB.idx["name"].keys())
    expected_names = {msg["name"] for msg in DATA}
    assert actual_names == expected_names


    

    #test_all_ages----------------------------
    actual_ages = set(DB.idx["age"].keys())
    expected_ages = {msg["age"] for msg in DATA if "age" in msg}
    assert actual_ages == expected_ages

    actual_num_uniq_ages = len(DB.idx["age"])
    expected_num_uniq_ages = len(set(x['age'] for x in DATA if "age" in x))
    assert actual_num_uniq_ages == expected_num_uniq_ages






    #test_all_strings----------------------------
    actual_strings = [x.msg['string'] for x in DB]
    expected_strings = [msg["string"] for msg in DATA]
    assert actual_strings == expected_strings







    #test_called_batman----------------------------
    all_batmans = DB.idx['name']['batman']
    all_batman_msgs = [x.msg for x in all_batmans]
    expected_batmans = [msg for msg in DATA if msg['name']=="batman"]
    assert all_batman_msgs == expected_batmans








    #test_age_25----------------------------
    all_age_25 = DB.idx['age'][25]

    all_age_25_msgs = [x.msg for x in all_age_25]
    expected_age_25 = [msg for msg in DATA if 'age' in msg and msg['age']==25]
    assert all_age_25_msgs == expected_age_25







    #test_age_ge_6_and_slice----------------------------
    def age_ge_6(age):
        return age >= 6
    #all_age_ge_6 = DB.idx['age'](age_ge_6)
    # Or
    all_age_ge_6 = DB.idx(age=age_ge_6)
    all_age_ge_6_msgs = [x.msg for x in all_age_ge_6]

    expected_ge_6 = [msg for msg in DATA if 'age' in msg and msg['age']>=6]
    assert all_age_ge_6_msgs == expected_ge_6







    #test_batman_in_string----------------------------
    def batman_in_str(msg):
        return "batman" in msg['string']

    all_with_batman_str = DB.stream(batman_in_str)
    all_with_batman_str_msgs = [x.msg for x in all_with_batman_str]
    expected_batman_str_msgs = [msg for msg in DATA if "batman" in msg['string']]
    assert all_with_batman_str_msgs == expected_batman_str_msgs






    #test_batman_in_string_and_age_25___or_name_superman----------------------------
    expected_data = [msg for msg in DATA if (("age" in msg and msg['age'] == 25) and "batman" in msg["string"]) or msg["name"] == "superman" ]

    def batman_in_str(msg):
        return "batman" in msg['string']
    def msg_age_eq_25(msg):
        return 'age' in msg and msg['age'] == 25

    # Method 1
    all_with_batman_str_and_age_25 = DB.stream(batman_in_str)(msg_age_eq_25)          #<--- Not using index
    name_supername = DB.idx['name']['superman']
    actual_msgs = [x.msg for x in all_with_batman_str_and_age_25 | name_supername]
    assert actual_msgs == expected_data
    print("Method 1: passed")

    # Method 2
    age_25 = DB.idx['age'][25]                                                      #<--- Using index
    with_batman_str = DB.stream(batman_in_str)                                        #<--- Not using index
    name_supername = DB.idx['name']['superman']
    actual_msgs = [x.msg for x in (age_25 & with_batman_str) | name_supername]
    assert actual_msgs == expected_data
    print("Method 2: passed")

    # Method 3
    age_25_with_batman_str = DB.idx['age'][25](batman_in_str)                      #<--- Only looking at messages that satisfy age. Faster
    name_supername = DB.idx['name']['superman']
    actual_msgs = [x.msg for x in age_25_with_batman_str | name_supername]
    assert actual_msgs == expected_data
    print("Method 3: passed")

    #Method 4
    age_25_with_batman_str = DB.stream(msg_age_eq_25)(batman_in_str)                 #<--- Not using index but at least using one to filter the other
    name_supername = DB.idx['name']['superman']
    actual_msgs = [x.msg for x in age_25_with_batman_str | name_supername]
    assert actual_msgs == expected_data
    print("Method 4: passed")

    # Method 5
    def age_eq_25(age):
        return age == 25
    age_25_with_batman_str = DB.idx['age'](age_eq_25)(batman_in_str)
    name_supername = DB.idx['name']['superman']
    actual_msgs = [x.msg for x in age_25_with_batman_str | name_supername]
    assert actual_msgs == expected_data
    print("Method 5: passed")

    # Method 6
    age_25_with_batman_str = DB.idx(age=age_eq_25)(batman_in_str)
    name_supername = DB.idx['name']['superman']
    actual_msgs = [x.msg for x in age_25_with_batman_str | name_supername]
    assert actual_msgs == expected_data
    print("Method 6: passed")

    # Method 7
    def msg_name_superman(msg):
        return msg['name'] == 'superman'
    everything = DB.idx(age=age_eq_25)(batman_in_str) | DB.stream(msg_name_superman)
    actual_msgs = [x.msg for x in everything]
    assert actual_msgs == expected_data
    print("Method 7: passed")









    #test_batman_in_string_minus_age25----------------------------
    expected_data = [msg for msg in DATA if "batman" in msg["string"] and not ("age" in msg and msg["age"]==25) ]

    def batman_in_str(msg):
        return "batman" in msg['string']

    everything = DB.stream(batman_in_str) - DB.idx['age'][25]
    actual_msgs = [x.msg for x in everything]
    assert actual_msgs == expected_data

