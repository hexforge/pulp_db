### # http://tomerfiliba.com/blog/Cartesian-Tree-Product/

# Objectives
#   * Query is lazy.  Only evaluates when repr, str, or asked.
#   * A standard Query relates to a key db.
#   * A streaming Query relates to the master db
#   * A virtual Query joins other Queries.  vQuery = (Query(view))&(Query(view))
#   * A query decomposes itself when involving many key_vals.


#db.idx.fields()
#db.idx['Last Price'].keys()     #  What Last Prices are there
#db.idx['Last Price'].values()   #  What messages have Last Price
#db.idx['Last Price']['Quote']   #  What message have Last Price =
#(db.idx['Last Price']['Quote']||db.idx['Last Price']['Trade'])[2]   #  What message have Last Price =

#db.stream.fields()
#db.stream['Last Price']


# A stream has lowest priority.  Indexed fields highest.
# Sparse always has higher priority than dense.
# Always chunk.  Data sets larger than memory.  Variable number of messages per chunk.
# Joining Queries together just sets up the plumbing. Evaluation gets the water flowing.
# 

#-----------------------------------------------
#Expressions are lazy and shouldn't need to be optimized by order
#
#Example queries:
#    Data.idx(symbol='frequent_sym')&Data.idx(symbol='rare_symbol') 
#    Shoudl evaluate at the same speed as
#    Data.idx(symbol='rare_symbol')&Data.idx(symbol='frequent_sym')
#
#-----------------------------------------------
#Things to think about.
#NullQueryResult, len = 0, nohing to iterate over.
#This is invalid db[1]&db(some_query).  This is valid db[1:1]&some_query.  
#Query results are allways in a container even if one result. Will get back [<msg1>] not <msg1>
#
#If we can't index everything. 
#If the data must be stored in raw format (no self explaining json)
#Then we have duplication between the index and the raw,
#
#Slight inconsistancy
#x['symbol'][by number] # This just gives the key??????
#x['symbol']['foo'] # This is a full query gives out messages??????

"""
Python interface to data store. Protype api discussion.

#######################################

Examples:

---------------------------------------------
# Open and write into a database

db = pulp_db('/path/to/db', 'w')
db.append(data, {'symbol': 'Spy', 'ip': '127.0.0.1'})   # Index is optional.
db.close()                                              #<--- Maybe split this up as it may take a while to merge ec.


# Alternative open and write into a database
with pulp_db('/path/to/db', 'w') as db:
    db.append(data, {data})

    ## What about one to many relationships. One message many symbols!!!!!!!!!!!!!
        #  Need to support this {'symbol': ['Spy', 'Foo']}

--------------------------------------------
# Open and read the flat file back from the db

with pulp_db('/path/to/db', 'r') as db:
    for x in db:
        print(x.msg)

---------------------------------------------
# First msg in playback

db[0].msg

---------------------------------------------
# First ten msg in playback

msgs = [x.msg for x in db[:10]]

--------------------------------------------
# List of indexed fields
fields = db.idx.keys()
OR
fields = db.idx.fields()

---------------------------------------------
# first symbol by alphabetical order

some_syms = db.idx[0]

---------------------------------------------
# Every 10th symbol

some_syms = db.idx[::10]

--------------------------------------------
# All symbols
syms = db.idx['symbol'].keys()
OR
syms = [sym for sym in db.idx['symbol']]

---------------------------------------------
# 50 random symbols

50_ran_sym = random.choice(db.idx['symbol'])

----------------------------------------------
# Top 50 symbols

len_syms = [(len(db.index['symbol'][sym]), sym) for sym in db.idx['symbol']]
sorted_len_syms = len_syms.sort()
top_50_syms = [sym for len, sym in sorted_len_syms[:50]]

---------------------------------------------
# Iterate over symbols

for sym in db.idx['symbol']:
     print(sym)
OR
for i in range(db.idx[symbol]):
    db.idx['symbol'].geti(i)

--------------------------------------------
# Every message for a particular symbol

for x in db.idx['symbol'][some_sym]:
     print(x.msg)

--------------------------------------------
# Every message for either symbol

s1_query = db.idx['symbol']['SPY']
s2_query = db.idx['symbol']['FOO']
for x in (s1_query or s2_query):
    print(x.msg)

---------------------------------------
# Every message for spy from 127.0.0.1

spy_query =  db.idx['symbol']['SPY']
ip_query = db.idx['ip']['127.0.0.1']
for x in (spy_query and ip_query)

---------------------------------------
#Everything but spy from 127.0.0.1

ip_query - spy_query 

---------------------------------------
Similar for xor (Everything but above)
ip_query ^ spy_query

Will steal most of these from set.

--------------------------------------------
# Number of messages in pb
len(db)

-------------------------------------------
# Number of symbols
len(db.idx['symbol'])

--------------------------------------------
# Number of spy messages
len(db.idx['symbol']['spy'])

--------------------------------------------
# Get previous message for a symbol
x = (db.idx['msg_type']['2'] & db.idx['symbol']['Spy'])[0]

db.idx['symbol']['Spy'].find(x.id).prev()

---------------------------------------------
# Just books. When should be get symbols and when should we get messages.

def get_books(symbol):
    return symbol.startswith('b')

for x in db.idx(symbol=get_books):
    print(x.msg)  # In order we got them.
OR
for x in db.idx.symbol(get_books):
    print(x.msg)
OR
db.idx['symbol'][sym1] or db.idx['symbol'][sym2] or db.idx['symbol'][sym3]
syms = [sym for sym in db.idx['symbol'] if get_books(sym)]
OR
query = None
for sym in syms:
    if query is None:
        query = db.idx['symbol'][sym]
    else:
        query |= db.idx['symbol'][sym]
for x in query:
    print(x.msg)

-----------------------------------------------
# Streaming query

def my_query(msg):
    return msg.blah == 11
       
for x in db.stream(my_query):
    print(x.msg)

-----------------------------------------------
Can slice results

db[:100]&db.index(symbol='Spy')  # Get any spy messages in the first hundred
db.idx(symbol='Spy')[:100]     # Get first 100 spy messages


-----------------------------------------------
All messages that have been index with some value of wSomeField
db.idx['wSomeField'].any()
OR
db.idx(wSomeField=lambda sym: True)

"""
import functools
import copy

from .msg import Msg

OR = 1
AND = 2
SUB = 3
XOR = 4

class FieldQuery(object):
    def __init__(self, fld_db, master_table):
        self.fld_db = fld_db
        self.fieldname = self.fld_db.fieldname
        self.master_table = master_table
        self.length = len(self.fld_db)

    def __repr__(self):
        return "<FieldQuery::{}, id={}, num_keys={}>".format(self.fieldname, id(self), len(self))

    def __len__(self):
        return self.length

    def __iter__(self):
        return (key for key in self.fld_db.keys())

    def __call__(self, selector):
        # What happens if we have too many values. 
        # We need a virtual limit, and temp persistence 
        # else we blow memory just with the starting points.

        selected_keys = [k for k in self.keys() if selector(k)]
        if not selected_keys:
            return None


        key_queries = []
        for key in selected_keys:
            key_query = self.lookup(key)
            key_queries.append(key_query)

        VirtualAddQuery = functools.partial(VirtualQuery, 
                                            operator=OR, 
                                            master_table=self.master_table)

        all_query = functools.reduce(VirtualAddQuery, key_queries)
        return all_query

    def __getitem__(self, key):
        return KeyQuery(self.fld_db.__getitem__(key),  master_table=self.master_table)

    def __contains__(self, x):
        return x in self.keys()

    def keys(self):
        nkeys = len(self.fld_db)
        return (self.fld_db.getkeyi(i) for i in range(nkeys))

    def any(self):
        return self(lambda key: True)

    def lookup(self, key):
        x = KeyQuery(self.fld_db.__getitem__(key),  master_table=self.master_table)
        #import pdb; pdb.set_trace()
        return x

    # Todo
    #def __and__(self, other):
    #    return VirtualQuery(self.any(), other.any(), AND, self.master_table)
    #
    #def __or__(self, other):
    #    return VirtualQuery(self.any(), other.any(), OR, self.master_table)
    #
    #def __sub__(self, other):
    #    return VirtualQuery(self.any(), other.any(), SUB, self.master_table)  
    #
    #def __xor__(self, other):
    #    return VirtualQuery(self.any(), other.any(), XOR, self.master_table)



#########################################################################################################


class Base(object):
    def __init__(self, master_table):
        self.length = None
        self.master_table = master_table

    ######### Required to implement
    def __next__(self):
        raise NotImplementedError("TODO: ABC this")

    def __prev__(self):
        raise NotImplementedError("TODO: ABC this")

    ########## Some Work required -----------------------
    def __call__(self, func):
        #print("am gere")
        #new_iterable = copy.deepcopy(self)
        return Filter(self, func)

    ########### Provided ---------------------------------
    def next(self):
        return self.__next__()

    def prev(self):
        return self.__prev__()

    def __and__(self, other):
        return VirtualQuery(self, other, AND, self.master_table)
    
    def __or__(self, other):
        return VirtualQuery(self, other, OR, self.master_table)

    def __sub__(self, other):
        return VirtualQuery(self, other, SUB, self.master_table)

    def __xor__(self, other):
        return VirtualQuery(self, other, XOR, self.master_table)

    def head(self):
        return self[0:10]

    def tail(self):
        num_msgs = len(self.msgrefs)
        return self[num_msgs-10: num_msgs]

    def make_msg(self, msg_id):
        if msg_id < 0:
            raise IndexError()
        return Msg(msg_id, master_table=self.master_table)


# This looks very very similar to the thing it wrapping.  Needless??
class KeyQuery(Base):
    def __init__(self, msgrefs, master_table):
        super(KeyQuery, self).__init__(master_table)
        self.query = msgrefs

    def __iter__(self):
        yield self[0]
        while True:
            yield self.next()
    
    def __getitem__(self, item):
        if isinstance(item, int):
            if item >= 0:
                return self.make_msg(self.query[item])
            else:
                assert(len(self) + item >= 0)
                return self.make_msg(self.query[len(self) + item])
        elif isinstance(item, slice):
            print("Warning dirty context possible here")
            return Slicer(self, item)
        else:
            raise NotImplementedError("Haven't coded this")

    def __prev__(self):
        try:
            msg = self.make_msg(self.query.prev())
        except IndexError:
            raise StopIteration() #from None
        return msg

    def __next__(self):
        try:
            msg = self.make_msg(self.query.next())
        except IndexError:
            raise StopIteration() #from None
        return msg

    def ge(self, ref):
        return self.make_msg(self.query.ge(ref))

    def le(self, ref):
        return self.make_msg(self.query.le(ref))

    # Extra
    def __len__(self):
        return len(self.query)

class StreamQuery(Base):
    """
    This is liked a merged Field and Key Query it holds state. 
    No need for seperation as only have indexs.
    """
    def __init__(self, master_table):
        super(StreamQuery, self).__init__(master_table)
        self.current_msg = None
        self.current_i = None

    def __getitem__(self, item):
        if isinstance(item, slice):
            return Slicer(self, item)
        
        assert isinstance(item, int)
        if item < 0:
            #print(item, len(self))
            if -item > len(self):
                raise IndexError("Index too big")
            item = len(self) + item
        if item >= len(self):
            raise IndexError("Index too big")

        if self.current_i != item:
            self.current_i = item
            self.current_msg = self.make_msg(self.current_i)
        return self.current_msg

    def __iter__(self):
        return self

    def __next__(self):
        if self.current_i is None:
            self.current_i = 0
            #print("hello", self.current_i)
        elif self.current_i <=  len(self.master_table) - 1:
            #print("beep")
            self.current_i += 1
            if self.current_i == len(self.master_table):
                raise StopIteration("Foobar")
        else:
            raise StopIteration("Foobar")

        msg = self.make_msg(self.current_i)
        self.current_msg = msg
        return self.current_msg
    
    def __prev__(self):
        if self.current_i is None:
            self.current_i = len(self.master_table) - 1
        elif self.current_i <= -1:
            raise StopIteration("Foobar")
        elif self.current_i <= 0:
            self.current_i -= 1
            raise StopIteration("Foobar")
        else:
            self.current_i -= 1
        
        msg = self.make_msg(self.current_i)
        self.current_msg = msg
        return msg

    def ge(self, index):
        if index >= len(self.master_table):  # No longer valid see slice.
            raise IndexError("Index={}, max={}".format(index, len(self)))
        elif index < 0:
            index = 0
        return self.make_msg(index)

    def le(self, index):
        if index >= len(self.master_table):  # No longer valid see slice.
            index = len(self.master_table) - 1 
        elif index < 0:
            raise IndexError("index={}, min={}".format(index, 0))
        return self.make_msg(index)

    #---- Extra
    def __len__(self):
        return len(self.master_table)

############################################
# A slice can be of anything. We may not know len etc.

class NEXT_EXHAUSTED(): pass
class PRE_EXHAUSTED(): pass
class NEW(): pass

STATE_NEW = NEW
STATE_NEXT_EXHAUSTED = NEXT_EXHAUSTED
STATE_PREV_EXHAUSTED = PRE_EXHAUSTED
RESET_STATES = {STATE_NEW, STATE_NEXT_EXHAUSTED, STATE_PREV_EXHAUSTED}

class Slicer(Base):
    def __init__(self, underlying, slicer):
        super(Slicer, self).__init__(underlying.master_table)
        self.underlying = underlying
        self.slice = slicer
        
        self.current_i = STATE_NEW  # curreth ith value yeilded
        self.current_p = None  # Position of underlying
        self.current_msg = None

        self.stop = self.slice.stop
        if self.slice.start is not None:
            self.start = self.slice.start
        else:
            self.start = 0

        if self.slice.step is not None:
            self.step = self.slice.step
        else:
            self.step = 1

    def __get_first(self):   #[-10::-2] [-10::2] [-10::-6::2]
        i = self.start
        while True:
            try:
                msg = self.underlying[i]  # What if there is no i = -10 [-10::2]
            except IndexError:
                if i * self.step > 0:     # Same sign
                    raise IndexError("Foobar")
                i += self.step
            else:
                break

        self.current_i = 0       # 0 is first from start
        self.current_p = i
        self.current_msg = msg
        return msg

    def __get_last(self):
        #print("what")
        if self.stop is None:
            self.__get_first()
            msg = self.current_msg
            i = self.current_p
            while True:
                try:
                    msg = self.underlying[i+self.step]  # What if [10::-2]
                except IndexError:
                    break
                i += self.step
        else:

            i = self.stop - self.step
            while True:
                try:
                    msg = self.underlying[i]  # What if there is no i = -10 [-10::2]
                except IndexError:
                    if i * self.step > 0:     # Same sign
                        raise IndexError("Foobar")
                    i += self.step
                else:
                    break
        
        self.current_i = -1       # -1 is first from end
        self.current_p = i
        self.current_msg = msg
        return self.current_msg

    def __getitem__(self, i):
        if i >= 0:
            if self.current_i in RESET_STATES:
                self.__get_first()
            p = self.current_p + self.step * (i - self.current_i)
        else:
            if self.current_i in RESET_STATES:
                self.current_p()
            p = self.current_p + self.step * (i - self.current_i)

        if self.stop is not None and (p >= self.stop):
            raise IndexError("out of range")
        if p < self.start:
            raise IndexError("out of range")

        self.current_msg = self.underlying[p]
        self.current_i = i
        self.current_p = p
        return self.current_msg

    def __iter__(self):
        self.current_i == STATE_NEW
        self.current_p == STATE_NEW
        return self 

    def __next__(self):
        #import pdb
        #pdb.set_trace()

        if self.current_i == STATE_NEXT_EXHAUSTED:
            raise StopIteration("Past limit")
        elif self.current_i in {STATE_PREV_EXHAUSTED, STATE_NEW}:
            try:
                msg = self.__get_first()
            except IndexError:
                raise StopIteration() #from None
            return msg

        if self.current_p == -1:
            self.current_i = STATE_NEXT_EXHAUSTED
            raise StopIteration("Hit limit")
        i = self.current_i + 1
        
        try:
            msg = self[i]
        except IndexError:
            self.current_i = STATE_NEXT_EXHAUSTED
            self.current_msg = None
            raise StopIteration() #from None
        else:
            self.current_i = i
            self.current_msg = msg
            return self.current_msg

    def __prev__(self):
        #print("ghelo", self.current_i)
        if self.current_i == STATE_PREV_EXHAUSTED:
            raise StopIteration("Past limit")
        elif self.current_i in {STATE_NEXT_EXHAUSTED, STATE_NEW}:
            try:
                msg = self.__get_last()
            except IndexError:
                raise StopIteration() #from None
            return msg
        
        if self.current_p == 0:
            self.current_i = STATE_PREV_EXHAUSTED
            raise StopIteration("Hit limit")
        i = self.current_i - 1
        
        #print("ghelo", i)
        try:
            msg = self[i]
        except IndexError:
            self.current_i = STATE_PREV_EXHAUSTED
            self.current_msg = None
            raise StopIteration() #from None
        else:
            self.current_i = i
            self.current_msg = msg
            return self.current_msg

    # Not coded yet
    def ge(self, index):  # We could go from current position. Optimisation easy.
        if self.current_i in RESET_STATES or self.current_msg.id > index:
            try:
                self.current_msg = self.__get_first()
            except IndexError:
                raise

        while True:
            if self.current_i == STATE_NEXT_EXHAUSTED:
                raise IndexError("Not Found")

            if self.current_msg.id > index:
                return self.current_msg
            elif self.current_msg.id < index:
                try:
                    self.current_msg = self.next()
                except StopIteration:
                    self.current_i = STATE_NEXT_EXHAUSTED;
                    raise IndexError() #from None
            else:
                return self.current_msg

    def le(self, index):  # We could go from current position. Optimisation easy.
        if self.current_i in RESET_STATES or self.current_msg.id > index:
            try:
                self.current_msg = self.__get_last()
            except IndexError:
                raise
        
        while True:
            if self.current_i == STATE_PREV_EXHAUSTED:
                raise IndexError("Not Found")
            if self.current_msg.id < index:
                return self.current_msg
            elif self.current_msg.id > index:
                try:
                    self.current_msg = self.prev()
                except StopIteration:
                    self.current_i = STATE_PREV_EXHAUSTED;
                    raise IndexError() #from None
            else:
                return self.current_msg


class Filter(Base):
    def __init__(self, underlying, filter):
        self.filter = filter
        self.current_i = STATE_NEW  # curreth ith value yeilded
        self.current_msg = None

        self.underlying = underlying
        super(Filter, self).__init__(self.underlying.master_table)  

    def __get_first(self):
        self.current_i = STATE_NEW

        i = 0
        while True:
            try:
                #print(i)
                msg = self.underlying[i]  # What if there is no i = -10 [-10::2]
            except IndexError:
                raise
            else:
                if self.filter(msg.msg):
                    break
            i += 1

        self.current_i = 0
        self.current_msg = msg
        return msg

    def __get_last(self):
        self.current_i = STATE_NEW

        i = -1
        while True:
            try:
                msg = self.underlying[i]  # What if there is no i = -10 [-10::2]
            except IndexError:
                raise
            else:
                if self.filter(msg.msg):
                    break
            i -= 1

        self.current_i = -1
        self.current_msg = msg
        return msg

    def __getitem__(self, i):
        if isinstance(i, slice):
            return Slicer(self, i)

        if i >= 0:
            if self.current_i in RESET_STATES:
                self.__get_first()

            while True:
                if self.current_i == i:
                    return self.current_msg
                elif self.current_i > i:
                    msg = self.prev()
                else:
                    msg = self.next()
        else:
            if self.current_i in RESET_STATES:
                self.__get_last()

            while True:
                if self.current_i == i:
                    return self.current_msg
                elif self.current_i > i:
                    msg = self.next()
                else:
                    msg = self.prev()
        
        raise IndexError("doooo")

    def __iter__(self):
        self.current_i == STATE_NEW
        return self

    def __next__(self):
        if self.current_i == STATE_NEXT_EXHAUSTED:
            raise StopIteration("Past limit")
        elif self.current_i in {STATE_PREV_EXHAUSTED, STATE_NEW}:
            try:
                msg = self.__get_first()
            except IndexError:
                raise StopIteration() #from None
            return msg
        

        while True:
            try:
                msg = next(self.underlying)
            except StopIteration:
                self.current_i == STATE_NEXT_EXHAUSTED
                self.current_msg = None
                raise
            if self.filter(msg.msg):
                self.current_i += 1
                self.current_msg = msg
                break

        return self.current_msg

    def __prev__(self):
        if self.current_i == STATE_PREV_EXHAUSTED:
            raise StopIteration()
        elif self.current_i in {STATE_NEXT_EXHAUSTED, STATE_NEW}:
            try:
                msg = self.__get_last()
            except IndexError:
                raise StopIteration() #from None
            return msg
        

        if self.current_i is None:
           self.current_i = - 1

        while True:
            try:
                msg = self.underlying.prev()
            except StopIteration:
                self.current_i == STATE_PREV_EXHAUSTED
                self.current_msg = None
                raise

            if self.filter(msg.msg):
                self.current_i -= 1
                self.current_msg = msg
                break

        return self.current_msg

    def ge(self, index):  # We could go from current position. Optimisation easy.
        if self.current_i in RESET_STATES or self.current_msg.id > index:
            try:
                self.current_msg = self.__get_first()
            except IndexError:
                raise

        while True:
            if self.current_i == STATE_NEXT_EXHAUSTED:
                raise IndexError("Not Found")
            
            if self.current_msg.id > index:
                return self.current_msg
            elif self.current_msg.id < index:
                try:
                    self.current_msg = self.next()
                except StopIteration:
                    self.current_i = STATE_NEXT_EXHAUSTED;
                    raise IndexError() #from None
            else:
                return self.current_msg

    def le(self, index):  # We could go from current position. Optimisation easy.
        if self.current_i in RESET_STATES or self.current_msg.id > index:
            try:
                self.current_msg = self.__get_last()
            except IndexError:
                raise
        
        while True:
            if self.current_i == STATE_PREV_EXHAUSTED:
                raise IndexError("Not Found")
            if self.current_msg.id < index:
                return self.current_msg
            elif self.current_msg.id > index:
                try:
                    self.current_msg = self.prev()
                except StopIteration:
                    self.current_i = STATE_PREV_EXHAUSTED;
                    raise IndexError() #from None
            else:
                return self.current_msg



class VirtualQuery(Base): 
    # Currently very limited as is a go forward kind of thing.

    def __init__(self, A, B, operator, master_table):
        super(VirtualQuery, self).__init__(master_table)
        func_map = {AND: and_func,
                    OR: or_func,
                    SUB: sub_func,
                    XOR: xor_func,
                    }

        self.A = A
        self.B = B
        self.operator = func_map[operator]
        self.current_i = STATE_NEW
        self.current_msg = None
        self.state = {"A": self.A,
                      "B": self.B,
                      "next_A": STATE_NEW, 
                      "next_B": STATE_NEW, 
                      } 

    def __get_first(self):
        self.state['next_A'] = self.A[0]
        self.state['next_B'] = self.B[0]
        
        try:
            self.current_msg, self.state = self.operator(self.state)
        except IndexError:
            self.current_i = STATE_NEXT_EXHAUSTED
            raise
        self.current_i = 0
        return self.current_msg

    def __get_last(self):
        self.state['next_A'] = self.A[-1]
        self.state['next_B'] = self.B[-1]
        
        try:
            self.current_msg, self.state = self.operator(self.state, reverse=True)
        except IndexError:
            self.current_i = STATE_PREV_EXHAUSTED
            raise
        self.current_i = -1
        return self.current_msg

    def __iter__(self):
        self.current_i = STATE_NEW
        
        try:
            self.current_msg = self.__get_first()
        except IndexError:
            self.current_i = STATE_NEXT_EXHAUSTED
            raise StopIteration #from None
        yield self.current_msg
        while True:
            self.current_msg = self.next()
            yield self.current_msg

    def __getitem__(self, item):
        if isinstance(item, slice):
            return Slicer(self, item)

        assert(isinstance(item, int))
        if self.current_i in RESET_STATES:
            if item >= 0:
                self.current_msg = self.__get_first()
            else:
                self.current_msg = self.__get_last()

        while True:
            if self.current_i == item:
                return self.current_msg
            elif self.current_i in RESET_STATES:
                raise IndexError("Bad")
            elif self.current_i > item:
                self.current_msg = self.prev()
            elif self.current_i < item:
                self.current_msg = self.next()
            else:
                raise NotImplementedError("WTF")

    def __next__(self):
        if self.current_i == STATE_NEXT_EXHAUSTED:
            raise StopIteration("Past limit")
        elif self.current_i in {STATE_PREV_EXHAUSTED, STATE_NEW}:
            try:
                self.current_msg = self.__get_first()
            except IndexError:
                raise StopIteration() #from None
            return self.current_msg

        if self.current_i == -1:
            self.current_i = STATE_NEXT_EXHAUSTED
            raise StopIteration()

        try:
            self.current_msg, self.state = self.operator(self.state)
        except IndexError:
            self.current_i = STATE_NEXT_EXHAUSTED;
            raise StopIteration() #from None
        else:
            self.current_i += 1
            return self.current_msg

    def __prev__(self):
        if self.current_i == STATE_PREV_EXHAUSTED:
            raise StopIteration("Past limit")
        elif self.current_i in {STATE_NEXT_EXHAUSTED, STATE_NEW}:
            try:
                self.current_msg = self.__get_last()
            except IndexError:
                raise StopIteration() #from None
            return self.current_msg

        if self.current_i == 0:
            self.current_i = STATE_PREV_EXHAUSTED
            raise StopIteration()

        try:
            self.current_msg, self.state = self.operator(self.state, reverse=True)
        except IndexError:
            self.current_i = STATE_PREV_EXHAUSTED
            raise StopIteration() #from None
        else:
            self.current_i += 1
            return self.current_msg

    def ge(self, x):
        if self.current_i in RESET_STATES or self.current_msg.id > index:
            try:
                self.current_msg = self.__get_first()
            except IndexError:
                raise

        while True:
            if self.current_i == STATE_NEXT_EXHAUSTED:
                raise IndexError("Not Found")

            if self.current_msg.id > index:
                return self.current_msg
            elif self.current_msg.id < index:
                try:
                    self.current_msg = self.next()
                except StopIteration:
                    self.current_i = STATE_NEXT_EXHAUSTED
                    raise IndexError() #from None
            else:
                return self.current_msg

    def le(self, x):
        if self.current_i in RESET_STATES or self.current_msg.id > index:
            try:
                msg = self.__get_last()
            except IndexError:
                raise
        
        while True:
            if self.current_i == STATE_PREV_EXHAUSTED:
                raise IndexError("Not Found")
            if self.current_msg.id < index:
                return self.current_msg
            elif self.current_msg.id > index:
                try:
                    self.current_msg = self.prev()
                except StopIteration:
                    self.current_i = STATE_PREV_EXHAUSTED
                    raise IndexError() #from None
            else:
                return self.current_msg


################################################################################################
################################################################################################

def get_next(thing, default):
    try:
        x = thing.next()
    except StopIteration:
        x = default
    return x

def get_prev(thing, default):
    try:
        x = thing.prev()
    except StopIteration:
        x = default
    return x

def ge_jump(thing, id):
    try:
        res = thing.ge(id)
    except IndexError:
        res = STATE_NEXT_EXHAUSTED
        raise
    return res

def le_jump(thing, id):
    try:
        res = thing.le(id)
    except IndexError:
        res = STATE_PREV_EXHAUSTED
        raise
    return res

def and_func(state, reverse=False):

    n = {"A": state['A'],
         "B": state['B'],
         "next_A": state['next_A'],
         "next_B": state['next_B'],
         }

    #print("A", n['A'], n['next_A'])
    #print("B", n['B'], n['next_B'])

    if not reverse:
        reset_states = {STATE_NEW, }
        stop_state = STATE_PREV_EXHAUSTED
        move_on = get_next
        jump = ge_jump
    else:
        reset_states = {STATE_NEW, }
        stop_state = STATE_NEXT_EXHAUSTED
        move_on = get_prev
        jump = le_jump

    while True:
        if n['next_A'] in reset_states:
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
        if n['next_A'] == stop_state:
            break

        if n['next_B'] in reset_states:
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
        if n['next_B'] == stop_state:
            break

        if n['next_A'].id == n['next_B'].id:
            msg = n['next_A']
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
            return msg, n

        elif n['next_A'].id > n['next_B'].id:
            n['next_B'] = jump(n['B'], id=n['next_A'].id)
        else:
            n['next_A'] = jump(n['A'], id=n['next_B'].id)

    raise IndexError("All done, One must be finished")

def or_func(state, reverse=False):

    n = {"A": state['A'],
         "B": state['B'],
         "next_A": state['next_A'],
         "next_B": state['next_B'],
         }

    #print("A", n['A'], n['next_A'])
    #print("B", n['B'], n['next_B'])

    if not reverse:
        reset_states = {STATE_NEW, }
        stop_state = STATE_PREV_EXHAUSTED
        move_on = get_next
        jump = ge_jump
    else:
        reset_states = {STATE_NEW, }
        stop_state = STATE_NEXT_EXHAUSTED
        move_on = get_prev
        jump = le_jump

    while True:
        if n['next_A'] in reset_states:
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
        if n['next_B'] in reset_states:
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)


        if n['next_A'] == n['next_B'] == stop_state:
            raise IndexError("All done, One must be finished")
        elif n['next_A'] == stop_state:
            msg = n['next_B']
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
            return msg, n
        elif n['next_B'] == stop_state:
            msg = n['next_A']
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            return msg, n
        
        if n['next_A'].id == n['next_B'].id:
            msg = n['next_A']
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
            return msg, n
        elif n['next_A'].id > n['next_B'].id:
            msg = n['next_B']
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
            return msg, n
        elif n['next_B'].id > n['next_A'].id:
            msg = n['next_A']
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            return msg, n
        else:
            raise NotImplementedError("WTF\n");

    raise NotImplementedError("All done, One must be finished")

def sub_func(state, reverse=False):

    n = {"A": state['A'],
         "B": state['B'],
         "next_A": state['next_A'],
         "next_B": state['next_B'],
         }

    #print("A", n['A'], n['next_A'])
    #print("B", n['B'], n['next_B'])

    if not reverse:
        reset_states = {STATE_NEW, }
        stop_state = STATE_PREV_EXHAUSTED
        move_on = get_next
        jump = ge_jump
    else:
        reset_states = {STATE_NEW, }
        stop_state = STATE_NEXT_EXHAUSTED
        move_on = get_prev
        jump = le_jump

    while True:
        if n['next_A'] in reset_states:
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
        if n['next_B'] in reset_states:
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)

        if n['next_A'] == stop_state:
            raise IndexError("done")
        elif n['next_B'] == stop_state:
            msg = n['next_A']
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            return msg, n
        
        if n['next_A'].id == n['next_B'].id:
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
        elif n['next_A'].id > n['next_B'].id:
            msg = n['next_B']
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
        elif n['next_B'].id > n['next_A'].id:
            msg = n['next_A']
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            return msg, n
        else:
            raise NotImplementedError("WTF\n");
    raise NotImplementedError("All done, One must be finished")

def xor_func(state, reverse=False):
    n = {"A": state['A'],
         "B": state['B'],
         "next_A": state['next_A'],
         "next_B": state['next_B'],
         }

    #print("A", n['A'], n['next_A'])
    #print("B", n['B'], n['next_B'])

    if not reverse:
        reset_states = {STATE_NEW, }
        stop_state = STATE_PREV_EXHAUSTED
        move_on = get_next
        jump = ge_jump
    else:
        reset_states = {STATE_NEW, }
        stop_state = STATE_NEXT_EXHAUSTED
        move_on = get_prev
        jump = le_jump

    while True:
        if n['next_A'] in reset_states:
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
        if n['next_B'] in reset_states:
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)

        if n['next_A'] == n['next_B'] == stop_state:
            raise IndexError("All done, One must be finished")
        elif n['next_A'] == stop_state:
            msg = n['next_B']
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
            return msg, n
        elif n['next_B'] == stop_state:
            msg = n['next_A']
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            return msg, n
        
        if n['next_A'].id == n['next_B'].id:
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
        elif n['next_A'].id > n['next_B'].id:
            msg = n['next_B']
            n['next_B'] = move_on(n['B'], STATE_PREV_EXHAUSTED)
            return msg, n
        elif n['next_B'].id > n['next_A'].id:
            msg = n['next_A']
            n['next_A'] = move_on(n['A'], STATE_PREV_EXHAUSTED)
            return msg, n
        else:
            raise NotImplementedError("WTF\n");
    raise NotImplementedError("All done, One must be finished")

################################################################################
############################################################################
# Old Code
#
#

#def merger(*the_data_lists):
#    """
#    When merging lists of data we want to go from sparse to dense.
#    """
#    data = [(len(data_list), data_list) for data_list in the_data_lists]
#    lengths = zip(*data)[0]
#    if min(lengths) > 1000000:
#        return intersection(*the_data_lists)
#    else:
#        #return intersection(*the_data_lists)
#        data.sort(key=lambda x: x[0],reverse=True)
#        data = zip(*data)[1]
#        return uber_wrap(*data)
#
#def double_dragon(list1, 
#                   list2, 
#                   find_func=bisect.bisect_left, 
#                   find_func2=bisect.bisect_right):
#    """
#    When merging two ordered data sets.  
#    Which end is more efficient to start from?  
#    Below we do both ends towards the middle.
#    
#    A: --  - --  --  - --  --  -
#    B:       ----
#
#    vs
#    A: ----  - --  -- - -- -------
#    B: ---  - -                --- 
#
#    vs 
#    A:  --- - - - - - -  ---- - - -
#    B: -- - ---   - -- - - - -- 
#
#    vs
#    A: - - - -  ----- -  - -- - --- 
#    B:    -             ----
#    """
#    find = find_func
#    find2 = find_func2
#    left_out = []
#    right_out = []
#    left_append = left_out.append
#    right_append = right_out.append
#    inx_l1 = inx_l2 = 0
#    inx_r1 = len(list1)
#    inx_r2 = len(list2)
#    
#    while inx_r1 > inx_l1 and inx_r2 > inx_l2:
#        #print counter.next()
#        
#        val_l1 = list1[inx_l1]
#        val_l2 = list2[inx_l2]
#        if  val_l1 == val_l2:
#            left_append(val_l1)
#            inx_l1 += 1
#            inx_l2 += 1 
#        elif val_l1 > val_l2:
#            inx_l2 = find(list2, val_l1, inx_l2, inx_r2)
#        else:
#            inx_l1 = find(list1, val_l2, inx_l1, inx_r1)
#        
#        if inx_r1 <= inx_l1 or inx_r2 <= inx_l2:
#        #if inx_l1 >= inx_r1 or inx_l2 >= inx_r2:
#            break
#        
#        val_r1 = list1[inx_r1-1]
#        val_r2 = list2[inx_r2-1]
#        if  val_r1 == val_r2:
#            right_append(val_r1)
#            inx_r1 -= 1
#            inx_r2 -= 1
#        elif val_r1 < val_r2:
#            inx_r2 = find2(list2, val_r1, inx_l2, inx_r2)
#        else:
#            inx_r1 = find2(list1, val_r2, inx_l1, inx_r1)
#            
#    right_out.reverse()
#    left_out.extend(right_out)
#    #print left_out
#    return left_out
