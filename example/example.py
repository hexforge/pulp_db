
import random

from pb_tools import get_decoder, parse_pb
import pulp_db


def decoder(bdata):
    meta_start = b"*M*E*T*A*S*T*A*R*T*"
    meta_end = b"*M*E*T*A*E*N*D*"
    meta, raw = bdata.split(meta_end, 1)
    h, hint, len, ip, port, time_str, time_num, thing, thing = meta.split(b'/')
    data = {"raw": raw,
            "hint": hint,
            "ip": ip,
            "time_str": time_str,
            "time_num": time_num}
    return data


DS_PATH = "example_ds"
PB_PATH = "example.pb"
# Write playback to db
with pulp_db.open(DS_PATH, "w") as db:
    for msg in parse_pb(PB_PATH):
        index = {}
        index['ip'] = msg['ip'] 
        index['port'] = msg['port']
        index['time'] = msg['time']
        
        raw = msg['msg']
        db.append(raw, index)

# Read and query db
with pulp_db.open(DS_PATH, "r", msg_loader=decoder) as db:

    #import pdb
    #pdb.set_trace()

    print("---First message---")
    print(db[0].msg)

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

    print("----Get a random ip")
    print(random.choice(list(db.idx['ip'].keys())))

    print("----Two Most common ip")
    freq_ip = [(len(db.idx['ip'][ip]), ip) for ip in db.idx['ip']]
    freq_ip.sort(reverse=True)
    print(freq_ip[:2])

    print("--All messages for ip=233.37.54.12---")
    count = 0
    seen = set()
    last_id = -1
    for x in db.idx['ip'][b'233.37.54.12']:
        print(x.msg)
        print(x.id)                         #<----- SHould be continous
        assert(x.id not in seen)
        assert(x.id > last_id)
        last_id = x.id
        seen.add(x.id)
        count += 0

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
        return int(ip[-1])%2==0

    for x in db.idx(ip=even_ip):
        print(x.msg)

    print("---Query unindexed field time_str 06:42:01.675")
    def time_str_filter(msg):
        return msg['time_str'] == b"06:42:01.675"
    res = db.stream(time_str_filter)
    for x in res:
        print(x.msg)

    print("--- Query mix of indexed and unindexed:: All from time_str 06:42:01.675 and time =1325626921.675784 ")
    def time_str_filter(msg):
        return msg['time_str'] == b"06:42:01.675"

    res = db.idx['time'][b'1325626921.675784'](time_str_filter)
    for x in res:
        print(x.msg)

    print("--- 2: Query mix of indexed and unindexed:: All from time_str 06:42:01.675 and time startswith=1325626921.6756 ")
    def time_str_filter(msg):
        return msg['time_str'] == b"06:42:01.675"
    def time_num_filter(time):
        return time.startswith(b"1325626921.6756")

    res = db.idx(time=time_num_filter)(time_str_filter)
    for x in res:
        print(x.msg)

    print("--- 3: Query mix of indexed and unindexed:: All from time_str 06:42:01.675 and time startswith=1325626921.6756")
    def time_str_filter(msg):
        return msg['time_str'] == b"06:42:01.675"
    def time_num_filter(msg):
        return msg['time_num'].startswith(b"1325626921.6756")

    res = db.stream(time_str_filter)(time_num_filter)
    for x in res:
        print(x.msg)

    print("--- 4: Using & Query mix of indexed and unindexed:: All from time_str 06:42:01.675 and time startswith=1325626921.6756")
    def time_str_filter(msg):
        return msg['time_str'] == b"06:42:01.675"
    def time_num_filter(time):
        return time.startswith(b"1325626921.6756")

    res = db.idx(time=time_num_filter) & db.stream(time_str_filter)
    for x in res:
        print(x.msg)
    


    # Below is currently bugged.
    #print("-Method 1-- All from time_str 06:42:01.675 and time_num ends even ")
    #def even_end_time_num(time_num):
    #    r = time_num[-1]%2 == 0
    #    print(time_num, r)
    #    return r
    #res = db.idx(time=even_end_time_num)
    #for x in res:
    #    print(x)





