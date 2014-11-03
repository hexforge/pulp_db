#!/usr/bin/env python

import sys
import __main__
import lib.pyt_factory as pyf
import lib.db_factory as dbf
import lib.file_handler as fh
import decimal
import simplejson as sjson
import bisect
import types as python_type
from pprint import pformat
import heapq
import array
#import tests.eg_tests as tests

#----------------------------
# Ideas
#----------------------------

#call a field_name as a function with message number to get its attribute

#Seperart length hash database to save opening the big one????

#Could make it smart, works out the length of the quere, 
#if too long, does it at run time, if short does a pivot around it :)

#DB generation, memory issues inefficent btree thing
#Numpy apply to all.
#elgence of api tests
#csv reducandy, why not fast
#graphs, times
#Data indepence

#filter
#global_accessors
#local_accessors
#prev_message
#next_message
#ordered list privotor

#--------------------------------------------------------
# Load message encoding here
#--------------------------------------------------------
encode_translate_dict  = fh.ENCODING_DICT
#encode_translate_dict['Decimal']=float # decimal.Decimal}
ENCODE_DICT = {}
ENCODE_DICT['regression'] = int
for name, enc_func_name in fh.yield_encoding_file():
    ENCODE_DICT[name]=encode_translate_dict[enc_func_name]
    
#print ENCODE_DICT
FIELD_ENCODING_DICT = {'Bidprice':decimal, 'wSrcTime':str}     # WTF <--
#if want name use .__name__

def invert_dict(old_dict):
#    tmp_dict = {}
    tmp_dict = {value:key for key, value in old_dict.iteritems()}
    old_dict.clear()
    old_dict.update(tmp_dict)
    del tmp_dict

FIELD_TRANS = fh.get_translator()
invert_dict(FIELD_TRANS)
#print FIELD_TRANS


print "WELCOME TO EYEBALL"

#------------------------------------------
# Regression message object
#------------------------------------------

class message_object(object):
    """This contains a regression message object, 
    
    This is called using the msg function, eg msg(109) for the 109th message
    n.b. blank fields are currently lost in the csv conversion, could add "Empty tag"
    """
    #could I use this as a despatcher
    def __init__(self, index, msg_dict):
        self.index = index
        self.attr_dict = {}
        for field_name, value in msg_dict.iteritems():
            # msg_list format = ["fieldname1=value1", "fieldname2=value2"]
            #field, value = pair.split('=')
            #print msg_dict
            field_name = FIELD_TRANS[int(field_name)]
            try:
                self.attr_dict[field_name] = ENCODE_DICT[field_name](value)    #add to attr_dict
                setattr(self, field_name, ENCODE_DICT[field_name](value))      #make stand alone attr
            except KeyError:
                raise
                #self.attr_dict[field_name] = str(value)
                #setattr(self, field_name, str(value))
        self.typ = self.attr_dict['Type']
        self.sym = self.attr_dict['Symbol']
        #del msg_dict #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    def fields(self):
        return self.attr_dict.keys()
    def has_fields(self, *required_fields):
        return set(required_fields) <= set(self.fields())
    def get_value(self, field_name):
        return self.attr_dict[field_name]
    def get_sym(self):
        return self.sym
    def get_type(self):
        return self.typ
    def values(self):
        return self.attr_dict.values()
    def __repr__(self):
        return "Message_object_{0}".format(self.index)
        #return repr(self.attr_dict)
    def __str__(self):
        return pformat(self.attr_dict, indent=4, width=40)
    def items(self):
        return self.attr_dict.items()
    #def __getattribute__(self, name):   With None returning messes up assignment??
    def __getattr__(self, attr_name):
        """Might take this out, is it useful??"""
        try:
            return object.__getattr__(self, attr_name)        #I don't understand this yet!
        except:                                               #rewrite this if you keep it in, and ffs add an exception
            print "No such field present {0}".format(attr_name)
            return None
    def __iter__(self):
        for key, value in self.attr_dict.iteritems():
            yield (key, value)

#-----------------------------------------------
# Master database object, interfaces with regression database
#-----------------------------------------------
class regression(object):
    #could make this a singleton??
    def __init__(self, regression_db, db_name='regression'):
        self.name = db_name
        self.encode = ENCODE_DICT[self.name]
        self.db_file = regression_db
        self.db = pyf.open_db(regression_db)
        self.length = len(self.db)
        self.low = 0
        self.high = self.length -1
        self.the_keys = None
    def __getitem__(self, key):
        if type(key) is python_type.SliceType:
            #So only encodes if not None which is accepted by get_list_slice
            start_val = key.start!=None and self.encode(key.start)
            end_val = key.stop!=None and self.encode(key.stop)
            if start_val == None:
                start_val = 0
            if end_val == None:
                end_val = self.high
            #coudl use read
            return [message_object(key, sjson.loads(item, use_decimal=True)) for item in self.db.iterrows(start_val, end_val)]
            #valid_values = get_list_slice_reg(self.the_keys, start_val, end_val)
            #return [message_object(key, sjson.loads(self.db[str(self.encode(key))], use_decimal=True)) for key in valid_values]
        elif self.low <= self.encode(key) <= self.high:
            return message_object(key, sjson.loads(self.db[int(key)], use_decimal=True))
        print "no such message, key ={0}, low={1}, high={2}".format(key, self.low, self.high)
        return None
    def __contains__(self, key):
        #as this is regression a sorted list could do this faster
        return binary_find(self.keys(), self.encode(key))
    def keys(self):
        if self.the_keys == None:
            return range(self.length)
    def __iter__(self):
        print "am here"
        x = 0
        for message in self.db.iterrows():
            x += 1
            yield message_object(x, sjson.loads(message, use_decimal=True))
    def __call__(self, *indexes):    #
        if len(indexes) > 1:
            print merger(*indexes), "MMMMM dunno"
            for number in merger(*indexes):
                yield self[number]
        elif len(indexes) == 1:
            try:
                iter_me = iter(indexes[0])
            except TypeError:
                yield self[indexes]
            else:
                for number in iter_me:
                    #print number, "erm"
                    yield self[number]
        else:
            for each in self.__iter__():
               yield each 

    
#msg(wBidPrice[5]|wBidPrice[20:30] and wSecStatus['Normal'])

#-----------------------------------------------
# Fieldname database object, interfaces with field reference databases
#-----------------------------------------------

class fields_obj(object):
    fields_dict = {}
    def __init__(self, field_name, field_db):
        #super(fields_obj, self).__init__(field_db, db_name=field_name)
        self.name = field_name
        self.encode = ENCODE_DICT[self.name]
        self.db_file = field_db
        self.db = dbf.open_db(field_db)
        fields_obj.fields_dict[field_name] = self
        setattr(sys.modules["__main__"], field_name, self)  #This is where Type Symbol etc 
        self.the_keys = None
        self.the_keys = self.keys()
        self.low = self.encode(self.the_keys[0])
        self.high = self.encode(self.the_keys[-1])
    def __getitem__(self, key):
        #print "am here"
        #print type(key)
        if type(key) is python_type.SliceType:
            #So only encodes if not None which is accepted by get_list_slice
            start_val = key.start and self.encode(str(key.start))
            end_val = key.stop and self.encode(str(key.stop))
            valid_values = get_list_slice_field(self.keys(), self.low, self.high, start_val, end_val)
            if valid_values == []:
                return index_obj(0, [], "empty", self.name) #This needs tested
            #gen_of_strings = (self.db[key] for key in valid_values)
            list_of_data = [array.array('L', self.db[key]) for key in valid_values]
            #[arragy.fromstring(stringy) for arragy,stringy in zip(list_of_data, gen_of_strings)]
            #list_of_data = [sjson.loads(self.db[key], use_decimal=True)[1] for key in valid_values]            
            joined_list = join_data(*list_of_data)
            length = len(joined_list)
            #print valid_values, "MOOO" ,self.low, self.high, start_val, end_val
            key_name = str(valid_values[0])+':'+str(valid_values[-1])
            return index_obj(length, joined_list, key_name, self.name)
        #else:
        #print self.name
        
        else:
            encoded_key = self.encode(str(key))
            #Below bug cause factor of a 1000 slow down
            #if encoded_key in self.keys(): #<Is this the problem!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            #if encoded_key in self.the_keys:
            if binary_find(self.the_keys, encoded_key):
                str_encoded_key = str(encoded_key)
                an_array = array.array('L')
                an_array.fromstring(self.db[str_encoded_key])
                length = len(an_array)
                return index_obj(length, an_array, encoded_key, self.name)
                #length, value_list = sjson.loads(self.db[str_encoded_key], use_decimal=True)
                #return index_obj(length, value_list, encoded_key, self.name)
    def __iter__(self):
        for x in dbf.get_all_records2(self.db_file):
            yield x[0], sjson.loads(x[1], use_decimal=True)[0]
        
        #for key in self.the_keys:
            #yield self[key].length
            
            #yield key
    def __call__(self, low=None, high=None):
        return get_list_slice_field(self.the_keys, self.low, self.high, low, high)
        #return self.keys()

    def keys(self):
        if self.the_keys == None:
            self.the_keys = [self.encode(key) for key in self.db]
            return self.the_keys
        else:
            return self.the_keys
    def key_set(self):
        return set(self.keys())
          
#--------------------------------------------
# Results from ref databases stored in an list like index_obj
#--------------------------------------------

class index_obj(object):
    def __init__(self, length, value_list, key, field_name):
        self.length = length
        self.value_list = value_list
        self.key = key
        self.field_name = field_name
        self.name = str(self.field_name)+'.'+str(self.key)
    def __repr__(self):
        return repr(self.value_list)
    def __contains__(self, x):
        return x in self.value_list
    def __getitem__(self, x):
        return self.value_list[x]
    def index(self, x):
        return self.value_list.index(x)
    def __add__(self, other_obj):
        other_vals = (x for x in other_obj)
        new_vals = join_data(self.value_list, other_vals)
        length = len(new_vals)
        return index_obj(length, new_vals, "joined_key", "joined_lists")
        #return self.value_list + y   #Need join instead here
    def __len__(self):
        return self.length
    def __iter__(self):
        for x in self.value_list:
            yield x
    def __or__(self, other):
        return self.__add__(other)
    def __and__(self, other):
        other_vals = other.value_list
        #new_vals = intersection(self.value_list, other_vals)
        new_vals = uber(self.value_list, other_vals)
        length = len(new_vals)
        return index_obj(length, new_vals, "and joined_key", "and joined_lists")
    def __sub__(self, other):
        other_vals = other.value_list
        new_vals = difference(self.value_list, other_vals)
        length = len(new_vals)
        return index_obj(length, new_vals, "sub joined_key", "sub joined_lists")
    def __xor__(self, other):
        other_vals = other.value_list
        new_vals = sym_difference(self.value_list, other_vals)
        length = len(new_vals)
        return index_obj(length, new_vals, "xor joined_key", "xor joined_lists")
    def prev(self, number):
        previous_index = binary_search_low_index(self.value_list, number) - 1
        if previous_index < 0:
            return None
        #print "previous_index", previous_index
        previous_value = self.value_list[previous_index]
        #print "previous_value", previous_value
        if previous_value < number:
            return previous_value
        else:
            return None
    def prevs(self, number):
        msg_list = get_index_slice(self.value_list, None, number)
        msg_list.reverse()
        return (msg[msg_num] for msg_num in msg_list)   #Dangerous
#-----------------------------------------------
# list object methods
#-----------------------------------------------

def binary_find(the_list, the_key):
    index = bisect.bisect(the_list, the_key) 
    if index > 0 and the_list[index -1] == the_key:
        return True
    else:
        return False

def binary_search_high_index(the_list, the_key):
    #print "high search", the_key
    #index = bisect.bisect(the_list, the_key) 
    #print "high result", index
    #return index
    index = bisect.bisect(the_list, the_key) 
    #if index > len(the_list):
    #    return index -1
    return index   
    
    
def binary_search_low_index(the_list, the_key):
    index = bisect.bisect_left(the_list, the_key) 
    #if index == len(the_list):
    #    return index -1
    return index
    #if index > 0 and the_list[index -1] == the_key:
    #    return index -1
    #FIX NEEDED HERE FOR low val not in list
    #else:
    #    return index

def join_data(*list_of_length_val_tuples):
    return [each_msg_num for each_msg_num in heapq.merge(*list_of_length_val_tuples)]

def get_list_slice_reg(the_list, low_val=None, high_val=None):
    #Warning this shifts messages.  Unconventional 
    # 0:0 gives nothing
    #so 0:1 = 1:1 = 1st message, 
    #2:2 = second message, 
    #2:3 = 2nd and 3rd, 
    #3:5, 3rd, 4th 5th    = [1,2,3,4,5] = 3,4,5, indexs = 2,3,4 
    if len(the_list) == 0:
        print "empty list, the_list"
        return []
    length = len(the_list)
    #print "detected slicey"
    if low_val == None:
        low_val = 0
    if high_val == None:
        high_val = length
        #print length, high_val, "value is None high"
    low_val = max(low_val-1, 0)
    if low_val > length -1 or high_val > length:
        print "WARNING Low or High val out of range {0}, {1}".format(low_val, high_val)
    return the_list[low_val:high_val]

def get_list_slice_field(the_list, list_low, list_high, low_val, high_val):
    #print low_val
    if len(the_list) == 0:
        print "empty list, the_list"
        sys.exit(1)
    #print "detected slicey"
    if low_val == None:
        low_val = list_low
    if high_val == None:
        high_val = list_high
    index_low = binary_search_low_index(the_list, low_val)
    index_high = binary_search_high_index(the_list, high_val)
    #print index_high, index_low, the_list, low_val, high_val
    #if index_high > index_low:
    return the_list[index_low:index_high]
    #else:
        #print the_list, index_low
    #    return the_list[index_low]

def get_index_slice(the_list, low, high):
    if len(the_list) == 0:
        print "empty list, the_list"
        sys.exit(1)
    if low:
        low = binary_search_low_index(the_list, low)   #Turns val into index else leaves None
    if high:
        high = binary_search_low_index(the_list, high) #Turns val into index else leaves None
    return the_list[low:high]

#def get_list_slice(list1, low, high):
#    print "detected slicey"
#    print "low", low
#    print "high", high
#    print "list", list
#    #Faster way to do this as is sorted list
#    if low!=None and high!=None:
#        return [item for item in list1 if low <= item <= high]
#    elif low!=None:
#        return [item for item in list1 if low <= item]
#    elif high!=None:
#        return [item for item in list1 if item <= high]
#    else:
#        return list1

#import itertools
#counter = itertools.count()

def uber(list1, list2, find_func=bisect.bisect_left, find_func2=bisect.bisect_right):
    """
    When merging two ordered data sets.  
    Which end is more efficient to start from?  Below we do both.
    
    set1: --  - --  --  - --  --  -
    set2:       ----

    vs 
     --- - - - - - -  ---- - - -
    -- - ---   - -- - - - -- 

    vs
    - - - -  ----- -  - -- - --- 
        -             ----
    """
    find = find_func
    find2 = find_func2
    left_out = []
    right_out = []
    left_append = left_out.append
    right_append = right_out.append
    inx_l1 = inx_l2 = 0
    inx_r1 = len(list1)
    inx_r2 = len(list2)
    
    while inx_r1 > inx_l1 and inx_r2 > inx_l2:
        #print counter.next()
        
        val_l1 = list1[inx_l1]
        val_l2 = list2[inx_l2]
        if  val_l1 == val_l2:
            left_append(val_l1)
            inx_l1 += 1
            inx_l2 += 1 
        elif val_l1 > val_l2:
            inx_l2 = find(list2, val_l1, inx_l2, inx_r2)
        else:
            inx_l1 = find(list1, val_l2, inx_l1, inx_r1)
        
        if inx_r1 <= inx_l1 or inx_r2 <= inx_l2:
        #if inx_l1 >= inx_r1 or inx_l2 >= inx_r2:
            break
        
        val_r1 = list1[inx_r1-1]
        val_r2 = list2[inx_r2-1]
        if  val_r1 == val_r2:
            right_append(val_r1)
            inx_r1 -= 1
            inx_r2 -= 1
        elif val_r1 < val_r2:
            inx_r2 = find2(list2, val_r1, inx_l2, inx_r2)
        else:
            inx_r1 = find2(list1, val_r2, inx_l1, inx_r1)
            
    right_out.reverse()
    left_out.extend(right_out)
    #print left_out
    return left_out

def uber_wrap(list1, *others):
    #Note I give this a list with smallest length furthest rught in others for speed
    #print "in uber_wrap"
    if len(others) > 1:
        return uber(list1, uber_wrap(*others))
    else:
        return uber(list1, *others)

def intersection(list_1, *other_lists):
    #print "in intersection"
    new_list = list(set(list_1).intersection(*other_lists))
    new_list.sort() #smallest first
    return new_list

def difference(list_1, *other_lists):
    print "in difference"
    new_list = list(set(list_1).difference(*other_lists))
    new_list.sort() #smallest first
    return new_list

def sym_difference(list_1, *other_lists):
    print "in sym_diff"
    new_list = list(set(list_1).symmetric_difference(*other_lists))
    new_list.sort()
    return new_list

def merger(*the_data_lists):
    data = [(len(data_list), data_list) for data_list in the_data_lists]
    lengths = zip(*data)[0]
    if min(lengths) > 1000000:
        return intersection(*the_data_lists)
    else:
        #return intersection(*the_data_lists)
        data.sort(key=lambda x: x[0],reverse=True)
        data = zip(*data)[1]
        return uber_wrap(*data)

#------------------------------
# Misc
#------------------------------

def print_msgnum(number):
    print msg[number]
    
def print_msgnums(*nums):
    for num in nums:
        print msg[num]

#--------------------------------
# Open regression file
#--------------------------------

print "getting regression file"
regression_file, field_files_dict = fh.get_dbs()
print "got regression file", regression_file

#Broke this need to work this better
msg = regression('regression')
for fieldname, filename in  field_files_dict:
    fields_obj(fieldname, filename)

#Super expensive
class lazy():
    def __init__(self):
        print "I only exist now"
        self.MSGS = index_obj(msg.length, array.array('L', range(msg.length)), 'ALL', 'ALL')

#-------------------------------------
# Global accessors
#-------------------------------------
#WHere does .Symbol etc come from??
def symbols():
    y = sys.modules["__main__"].Symbol
    x = [each_symbol for each_symbol in y.keys()]
    x.sort()
    for each_symbol in x:
        print each_symbol

def types():
    y = sys.modules["__main__"].Type
    x = [each_type for each_type in y.keys()]
    x.sort()
    for each_type in x:
        print each_type

def fields():
    x = [each_field for each_field in fields_obj.fields_dict]
    x.sort()
    for each_field in x:
        print each_field

def values(field_name = 'ALL'):
    the_dict = fields_obj.fields_dict
    for each_field in the_dict:
        print "FIELD_NAME = {0}".format(each_field)
        print the_dict[each_field]

FIELDS = [each_field for each_field in fields_obj.fields_dict]
FIELDS.sort()
TYPES = [each_symbol for each_symbol in sys.modules["__main__"].Type.keys()]
TYPES.sort()
SYMBOLS = [each_symbol for each_symbol in sys.modules["__main__"].Symbol.keys()]
SYMBOLS.sort()


#--------------------------
# Useful functions for testing
#--------------------------

def assert_approx_equal(value_one, value_two, dec_places=1, error_message='Not equal'):     
    """Checks if two values are apprix equal, returns true/false.
    
    Work in progress."""   
    truth = value_one == -0.2*dec_places < value_two or round(value_one, dec_places) - round(value_two, dec_places) < 0.2*dec_places
    if not truth:
        print error_message, value_one, value_two
    return truth

import datetime
def time_converter(the_string):
    """Funtcion for converting time strings so they can be checked.
    
    Takes in time of the form 13:15:45.045 or 13:15:45 
    and converts it to a datetime.time tuple thing."""
    new_string = the_string.split(':')
    if '.' in new_string[-1]:
        new_end = (new_string.pop()).split('.')
        new_end[-1] = new_end[-1]+'000'
        new_string.extend(new_end)
    time_tuple = tuple([int(element) for element in new_string])
    return datetime.time(*time_tuple)    

def the_date_converter(the_string):
    """Function for converting date strings so they can be checked.
    
    Takes in dates of the form 2010/03/09 and converts them
    into an datetime.datetime object"""
    return datetime.datetime.strptime(the_string,'%Y/%m/%d')
    #tuple([int(item) for item in list(itertools.chain(*[item.split('.') for item in x.split(':')]))])

#-------------------------------
# Easy Example tests
#------------------------------

#top50 symbols
def give_me_sym_by_pop():
    x = sys.modules["__main__"].Symbol
    z =  [(x[each_symbol].length, each_symbol) for each_symbol in x()]
    z.sort()
    for length, key in z:
        print key, length
    print "number of symbols =", len(sys.modules["__main__"].Symbol())

def graph_50():
    x = sys.modules["__main__"].Symbol
    z =  [(x[each_symbol].length, each_symbol) for each_symbol in x()]
    z.sort()
    z = z[-50:]
    #s = dict(z)
    #plt.interactive(True)
    for i, key, in enumerate(z): 
        plt.bar(i, key[0])
    print zip(*z)[1]
    plt.xticks(np.arange(len(z)), zip(*z)[1])
    plt.show()
    print "number of symbols =", len(sys.modules["__main__"].Symbol())

def give_me_sym_by_pop2():
    z = [(length, key) for key, length in sys.modules["__main__"].Symbol]
    z.sort()
    for length, key in z:
        print key, length
    print "number of symbols =", len(sys.modules["__main__"].Symbol())

x = sys.modules["__main__"]


def check_summary_books():
    number_of_fails = 0
    number_of_passes = 0
    for sym in x.Symbol.the_keys:
        if sym.startswith("s"):
            for message in msg(x.Type['Update']&x.Symbol[sym]):
                ask_list = [decimal.Decimal(getattr(message, "wBestAskPrice"+str(n))) for n in range(1,11) if decimal.Decimal(getattr(message, "wBestAskPrice"+str(n)))>0]
                bid_list = [decimal.Decimal(getattr(message, "wBestBidPrice"+str(n))) for n in range(1,11) if decimal.Decimal(getattr(message, "wBestBidPrice"+str(n)))>0]
                
                sorted_ask = sorted(ask_list)
                sorted_bid = sorted(bid_list, reverse=True)
                
                if sorted_ask!=ask_list or sorted_bid!=bid_list:
                    print message
                    print "ask_unsorted", ask_list
                    print "ask_sorted", sorted_ask
                    print "bid_unsorted", bid_list
                    print "bid_sorted", sorted_bid
                    print "FAIL"
                    number_of_fails  +=1
                else:
                    print "PASS"
                    number_of_passes += 1
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails

def no_quotes_during_closed():
    Quote_messages = x.Type['Quote']
    Closed_messages = x.wSecurityStatus['Closed']
    if Quote_messages and Closed_messages:
        if len(Quote_messages&Closed_messages) == 0:
            print "Test passed"
        else:
            print "Test Fail"
    else:
        print "None somewhere: Quote=None", Quote_messages==None, "Closed=None", Closed_messages==None

def Trades_only_during_Normal():
    Trade_messages = x.Type['Trade']
    Normal_messages = x.wSecurityStatus['Normal']
    if Trade_messages and Normal_messages:
        if len(Trade_messages-Normal_messages) == 0:
            print "Test passed"
        else:
            print "Found trades in ", set([message.wSecurityStatus for message in msg(x.Type['Trade'])])
            print "Test Fail"
    else:
        print "None somewhere: Trade=None", Trade_messages==None, "Normal=None", Normal_messages==None

def Prices_less_than_magic():
    All_passed = True
    names = ['wBidPrice', 'wAskPrice', 'wTradePrice', 'wAskHigh', 'wAskLow', 'wBidHigh', 'wBidLow']
    price_fields = [x.wBidPrice, x.wAskPrice, x.wTradePrice, x.wAskHigh, x.wAskLow, x.wBidHigh, x.wBidLow]
    for name, each_field in zip(names, price_fields):
        print "testing {0}".format(name)
        if each_field.the_keys[-1] >= 999999:
            print "FAIL, {0} Price:".format(name), each_field[999999:]
            All_passed = False
    if All_passed:
        print "ALL PASSED < 999999", names
    else:
        print "TEST FAIL"
        
def Prices_not_negative():
    All_passed = True
    names = ['wBidPrice', 'wAskPrice', 'wTradePrice', 'wAskHigh', 'wAskLow', 'wBidHigh', 'wBidLow']
    price_fields = [x.wBidPrice, x.wAskPrice, x.wTradePrice, x.wAskHigh, x.wAskLow, x.wBidHigh, x.wBidLow]
    for name, each_field in zip(names, price_fields):
        if each_field.the_keys[0] < 0:
            print "FAIL, {0} Price:".format(name)
            for message in msg(each_field[:-0.00001]):
                print message
            All_passed = False
        else:
            print "PASSED {0}".format(name)
    if All_passed:
        print "ALL PASSED Prices > 0", names
    else:
        print "TEST FAIL"

def date_fields_not_EPOCH():
    All_passed = True
    names = ['wTradeDate', 'wQuoteDate', 'wExpirationDate']
    date_fields = [x.wTradeDate, x.wQuoteDate, x.wExpirationDate]
    for name, each_field in zip(names, date_fields):
        if each_field.the_keys[0].startswith('19'):
            print "FAIL, {0} Price:".format(name), each_field.the_keys[0]
            All_passed = False
        else:
            print "PASSED {0}".format(name)
    if All_passed:
        print "PASSED date_fields not EPOCH", names
    else:
        print "TEST FAIL"   
    
def Required_fields_Quotes():
    All_passed = True
    req_fields = ['', '', '']
    for message in msg(Type['Quote']):
        if not message.has_fields(req_fields):
            All_passed = False
            print "FAIL: message missing one or more of", req_fields
            print message
    if All_passed:
        print "PASSED all quotes have required fields"
    else:
        print "TEST FAIL"

def Required_fields_Trades():
    All_passed = True
    req_fields = ['', '', '']
    for message in msg(Type['Trade']):
        if not message.has_fields(req_fields):
            All_passed = False
            print "FAIL: message missing one or more of", req_fields
            print message
    if All_passed:
        print "PASSED all trades have required fields"
    else:
        print "TEST FAIL"

def test_wVwap():
    number_of_fails = 0
    number_of_passes = 0
    for message in msg(x.Type['Trade']):
        if message.has_fields('wTotalVolume', 'wTotalValue', 'wVwap'):
            cal_wVwap = message.wTotalValue/message.wTotalVolume
            if not assert_approx_equal(cal_wVwap, message.wVwap, 2):
                print "FAIL:", message.wVwap, cal_wVwap
                print "ROUNDED:", round(message.wVwap, 2), round(cal_wVwap, 2)
                number_of_fails += 1
            else:
                print "PASS:", message.wVwap, cal_wVwap
                number_of_passes += 1
        else:
            print "No such fields"
            print message
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails


def entitlement_code_present_and_static():
    number_of_fails = 0
    number_of_passes = 0
    for sym in x.Symbol.the_keys:
        LastwEntitleCode = None
        for message in msg(x.Symbol[sym]):
            if message.has_fields('wEntitleCode'):
                wEntitleCode = message.wEntitleCode
                if not LastwEntitleCode:
                    LastwEntitleCode = wEntitleCode
                elif LastwEntitleCode == wEntitleCode:
                    print "PASS", message.index, LastwEntitleCode, wEntitleCode
                    number_of_passes += 1
                else:
                    print "FAIL",  message.index, LastwEntitleCode, wEntitleCode
                    number_of_fails += 1
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails


def wInstrumentType_present_and_static():
    number_of_fails = 0
    number_of_passes = 0
    for sym in x.Symbol.the_keys:
        LastwInstrumentType = None
        for message in msg(x.Symbol[sym]):
            if message.has_fields('wInstrumentType'):
                wInstrumentType = message.wInstrumentType
                if not LastwInstrumentType:
                    LastwInstrumentType = wInstrumentType
                elif LastwInstrumentType == wInstrumentType:
                    print "PASS", message.index, LastwInstrumentType, wInstrumentType
                    number_of_passes += 1
                else:
                    print "FAIL",  message.index, LastwInstrumentType, wInstrumentType
                    number_of_fails += 1
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails

def wPubId_present():
    num_of_message_without_pubid = 0
    temp = lazy()
    for message in msg(temp.MSGS-x.wPubId[:]):
    #for message in msg(x.Type[:]-x.wPubId[:]):
        num_of_message_without_pubid += 1
        print "FAIL", repr(message)
    if num_of_message_without_pubid == 0:
        print "ALL PASSED"
    else:
        print "found {0} messages without wPubId".format(num_of_message_without_pubid)
    del temp

def wPubId_static():
    number_of_fails = 0
    number_of_passes = 0
    LastwPubId = None
    for message in msg(x.wPubId[:]):
        wPubId = message.wPubId
        if not LastwPubId:
            LastwPubId = wPubId
        elif LastwPubId == wPubId:
            print "PASS", message.index, LastwPubId, wPubId
            number_of_passes += 1
        else:
            print "FAIL",  message.index, LastwPubId, wPubId
            number_of_fails += 1
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails
        
def trade_price_between_bid_and_ask():
    number_of_fails = 0
    number_of_passes = 0
    for message in msg(x.Type['Trade']):
        if message.has_fields('wBidPrice', 'wAskPrice', 'wTradePrice'):
            wBidPrice = message.wBidPrice
            wTradePrice = message.wTradePrice
            wAskPrice = message.wAskPrice
            
            if wBidPrice and wTradePrice and wAskPrice:
                if wBidPrice <= wTradePrice <= wAskPrice:
                    number_of_passes += 1
                    #print "PASS", message.index, wBidPrice, wTradePrice, wAskPrice
                else:
                    number_of_fails += 1
                    print "FAIL", message.index, wBidPrice, wTradePrice, wAskPrice
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails   

def total_value_increasing_correctly():
    """Test: total value increases by trade price * size.
    """
    number_of_fails = 0
    number_of_passes = 0
    for sym in x.Symbol.the_keys:
        LastwTotalValue = 0
        for message in msg(x.Symbol[sym]):
        #for message in msg(x.Type['Trade']&x.symbol[sym]):
            if message.has_fields('wTotalValue', 'wTradeVolume', 'wTradePrice'):
                wTotalValue = message.wTotalValue
                wTradeVolume = message.wTradeVolume
                wTradePrice = message.wTradePrice
                if wTotalValue and wTradeVolume and wTradePrice: 
                    if not LastwTotalValue:
                        LastwTotalValue = wTotalValue
                        continue
                    cal_wTotalValue_delta = wTotalValue - LastwTotalValue
                    cal_Trade_Volume_delta = wTradeVolume * wTradePrice
                    if not assert_approx_equal(cal_wTotalValue_delta, cal_Trade_Volume_delta, dec_places=2):
                        print "#"*40
                        print "FAIL, total value bad increase"
                        print "Testing symbol=", sym
                        print ' message_number= ', message.index
                        print 'wTotalValue', wTotalValue
                        print "LastwTotalValue", LastwTotalValue
                        print 'cal_ wTotalValue - LastwTotalValue', cal_wTotalValue_delta
                        print 'wTradeVolume', wTradeVolume
                        print 'wTradePrice', wTradePrice
                        print 'cal_wTradeVolume * wTradePrice', cal_Trade_Volume_delta
                        print "#"*40
                        number_of_fails += 1
                    else:
                        print "PASS: total value OK", "Testing symbol=", sym, ' message_number ', message.index, cal_wTotalValue_delta, '=', cal_Trade_Volume_delta, 'wTradeVolume', wTradeVolume, 'wTradePrice', wTradePrice, 'LastwTotalValue', LastwTotalValue,'Current wTotalValue', wTotalValue
                        number_of_passes += 1
                    LastwTotalValue = wTotalValue
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails  


def bidhigh_bidlow_rightway_around():
    number_of_fails = 0
    number_of_passes = 0
    for message in msg:
        if message.has_fields('wBidLow', 'wBidHigh'):
            wBidLow = message.wBidLow
            wBidHigh = message.wBidHigh
            
            if wBidLow and wBidHigh:
                if wBidLow <= wBidHigh:
                    number_of_passes += 1
                    print "PASS", message.index, wBidLow, wBidHigh
                else:
                    number_of_fails += 1
                    print "FAIL", message.index, wBidLow, wBidHigh
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails   

def bidhigh_bidlow_rightway_around2():
    number_of_fails = 0
    number_of_passes = 0
    for message in msg(x.wBidLow[:]&x.wBidHigh[:]):
        wBidLow = message.wBidLow
        wBidHigh = message.wBidHigh
        if wBidLow and wBidHigh:
            if wBidLow <= wBidHigh:
                number_of_passes += 1
                print "PASS", message.index, wBidLow, wBidHigh
            else:
                number_of_fails += 1
                print "FAIL", message.index, wBidLow, wBidHigh
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails

def askhigh_asklow_rightwar_around():
    number_of_fails = 0
    number_of_passes = 0
    for message in msg:
        if message.has_fields('wAskLow', 'wAskHigh'):
            wAskLow = message.wAskLow
            wAskHigh = message.wAskHigh
            
            if wAskLow and wAskHigh:
                if wAskLow <= wAskHigh:
                    number_of_passes += 1
                    print "PASS", message.index, wAskLow, wAskHigh
                else:
                    number_of_fails += 1
                    print "FAIL", message.index, wAskLow, wAskHigh
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails   


def sec_check():
    number_of_fails = 0
    number_of_passes = 0
    bad = 0
    for sym in x.Symbol.the_keys:
        if sym.startswith('SECURITY_STATUS'):
            for message in msg(x.Symbol[sym]):
                
                message_symbol = message.wIssueSymbol+'.'+message.wPartId
                new_sec_status = message.wSecurityStatus
                new_sec_org = message.wSecurityStatusOrig
                new_sec_time = message.wSecurityStatusTime
                
                SEC_fields = (message_symbol, new_sec_status, new_sec_org, new_sec_time)
                index = message.index
                prev_msg = x.Symbol[message_symbol].prev(index)
                if prev_msg == None:
                    print "No previous message", message_symbol, index
                    bad += 0
                
                prev_message = msg[prev_msg]
                PREV_fields = (prev_message.sym, prev_message.wSecurityStatus, prev_message.wSecurityStatusOrig, prev_message.wSecurityStatusTime)
                if SEC_fields == PREV_fields:
                    number_of_passes += 1
                    print "OK, SEC=", index, SEC_fields, "PREV=",prev_msg, PREV_fields
                else:
                    number_of_fails += 1
                    print "FAIL, SEC=", index, SEC_fields, "PREV=",prev_msg, PREV_fields
                
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails
    print "Bad tests", bad


#[message.wTradePrice for message in (Type['Trade']&symbol['ES0113860A34.XSEQ']).prevs(179764)]
def test_tick_direction():
    number_of_fails = 0
    number_of_passes = 0
    unknown = 0
    #possible_ticks = ['+', '-', '0+', '0-']
    
    List_tradeprice_msgs = x.wTradePrice[:]&x.Type['Trade']
    
    #List_tradeprice_msgs = x.Type['Trade']

    for sym in x.Symbol.the_keys:
        LastTradePrice = None
        the_sym_tradeprice_story = List_tradeprice_msgs&x.Symbol[sym]
        for message in msg(the_sym_tradeprice_story):
            if message.has_fields('wTradeTick'):
                wTradePrice = message.wTradePrice
                wTradeTick = message.wTradeTick
                if not LastTradePrice:
                    LastTradePrice = wTradePrice
                    LastTradeIndex = message.index
                else:
                    if wTradePrice > LastTradePrice:
                        if wTradeTick == '+':
                            number_of_passes += 1
                            print 'PASS', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym
                        else:
                            number_of_fails += 1
                            print 'FAIL', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym
                    elif wTradePrice < LastTradePrice:
                        if wTradeTick == '-':
                            number_of_passes += 1
                            print 'PASS', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym
                        else:
                            number_of_fails += 1
                            print 'FAIL', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym                 
                    else:
                        if wTradeTick in ['0+', '0-']:
                            cur_index = message.index
                            Last_changed_price = None
                            for prev_message in the_sym_tradeprice_story.prevs(cur_index):
                                if prev_message.wTradePrice != wTradePrice:
                                    Last_changed_price = prev_message.wTradePrice
                                    break
                            if Last_changed_price:
                                if wTradePrice > Last_changed_price:
                                    if wTradeTick == '0+':
                                        number_of_passes += 1
                                        print 'PASS', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym, Last_changed_price
                                    else:
                                        number_of_fails += 1
                                        print 'FAIL', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym, Last_changed_price
                                elif wTradePrice < Last_changed_price:
                                    if wTradeTick == '0-':
                                        number_of_passes += 1
                                        print 'PASS', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym, Last_changed_price
                                    else:
                                        number_of_fails += 1
                                        print 'FAIL', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym, Last_changed_price
                            else:
                                unknown += 1
                                print '#'*40
                                print "couldn't find any previous prices different"
                                print "!!!! Openprice =", prev_message.wOpenPrice, 
                                print "!!!! TradePrice = ", wTradePrice, 
                                print "!!!! Tick =", wTradeTick
                                print "!!!! Symbol = ", sym
                                print "!!!! Msg_num =", message.index
                                print '#'*40
                        else:
                            number_of_fails += 1
                            print 'FAIL', message.index, LastTradePrice, wTradePrice, wTradeTick, message.index, LastTradeIndex, message.sym                      
                        
                    LastTradePrice = wTradePrice
                    LastTradeIndex = message.index
            else:
                if not message.wTradeCount == 1:   
                    number_of_fails += 1
                    print "FAIL no tick", message.index   
    if number_of_fails == 0:
        print "ALL PASSED OK" 
    else:
        print "Some fails"
    print "number of passes =", number_of_passes
    print "number of fails =", number_of_fails
    print "number of unknown =", unknown



#----------------------------
# Highly experiemntal features
#----------------------------


#This suffers from get_list_slice_reg index issues.  not alined!!!!!!!!!!!!!!!!!!!!!

class msg_cache(object):
    """dict like object"""
    def __init__(self, initial=None):
        self.cache = {}
        if initial != None:
            self.update(initial)
            #get it's 
            #elif tuple of my index and intersection number
        
        self.stringy = None
        
        #All three of these must be populated
        self.msg_sequence = None
        self.index = None
        self.stringy = None
        #order_as_come = ordered_dictionary :) so show on screen in same place
        
    def update(self, msg_object):
        if isinstance(msg_object, message_object):
            print #*50
            print "updating with", msg_object.attr_dict
            print "updating from", self.cache
            print "updating to"
            self.update(msg_object.attr_dict)
            self.stringy = "I am a cache with value set"
        elif type(msg_object) == python_type.DictType:
            self.cache.update(msg_object)  
            self.stringy = "I am a cache with value set"
        elif isinstance(msg_object, index_obj):
            self.set(msg_object)
            self.stringy = "I am a index_obj with self.index:", self.index
        else:
            try:
                each_msg_ok = (isinstance(each_msg, message_object) for each_msg in msg_object)
                #alternative methods
                #x = iter(msg_dict)
                #or
                #(e for e in msg_dict)
                
                #import collections
                #if isinstance(theElement, collections.Iterable):
                #    # iterable
                #else:
                #    # not iterable
                
            except TypeError:# IteratorError:
                print "Not updatable"

            else:
                self.clear()
                if all(each_msg_ok):
                    for each_message in msg_object:
                        self.update(each_message)
                else:
                    print "not all message objects"
                self.stringy = "I am a index_obj with self.index:", self.index
                
    def clear(self):
        print "clearing cache"
        self.cache = {}
        self.msg_sequence = None
        self.index = None

    def __add__(self, other_obj):
        self.update(self, other_obj)
        
    def check_seq(self):
        return self.msg_sequence != None  
    
    def __getitem__(self, req_index):
        if not self.check_seq():
            print "thisin isn't sequencable"
            return
        #######################################
        if self.index == None:
            fields_to_apply = get_list_slice_reg(self.msg_sequence, high_val = req_index + 1)
            print fields_to_apply, isinstance(self.msg_sequence, index_obj)
            for each_update in fields_to_apply:
                self.update(each_update)
            self.index = min(req_index, len(self.msg_sequence) -1)
            return self.cache
        else:
            if self.index < req_index:
                fields_to_apply = get_list_slice_reg(self.msg_sequence, self.index+1, req_index + 1)
                z = [self.update(each_update) for each_update in fields_to_apply]
                if z:
                    self.index = req_index
                return self.cache                
            elif self.index > req_index:
                self.clear()
                return self.__getitem__(req_index)

    def goto(self, msg_num):
        if not self.check_seq():
            print "thisin isn't sequencable"
            return
        #######################################       
        the_index = binary_search_low_index(self.msg_sequence, msg_num)
        return self.__getitem__(the_index)
        
    def set(self, message_sequence, msg_num=None):
        #if not self.check_seq():
        #    print "thisin isn't sequencable"
        #    return
        ######################################
        if isinstance(message_sequence, index_obj):
            #self.msg_sequence = message_sequence
            self.clear()
            self.msg_sequence = map(msg.__getitem__, message_sequence.value_list)
        else:
            try:
                each_msg_ok = (isinstance(each_msg, message_object) for each_msg in message_sequence)
                #alternative methods
                #x = iter(msg_dict)
                #or
                #(e for e in msg_dict)
                
                #import collections
                #if isinstance(theElement, collections.Iterable):
                #    # iterable
                #else:
                #    # not iterable
                
            except TypeError:# IteratorError:
                print "Not updatable"

            else:
                self.clear()
                if all(each_msg_ok):
                    for each_message in message_sequence:
                        self.update(each_message)
                else:
                    print "not all message objects"
                self.stringy = "I am a index_obj with self.index:", self.index
        if self.msg_sequence:
            self.stringy = "I am a index_obj with self.index:", self.index
            if msg_num != None:
                return  self.goto(self, msg_num)
        
    def next(self):
        if not self.check_seq():
            print "thisin isn't sequencable"
            return
        #######################################
        if self.index == None:
            return self.__getitem__(0)
        elif self.index < len(self.msg_sequence) - 1:
            return self.__getitem__(self.index +1)
        else:
            print "outta messages"
            return {}
            
        
    def prev(self):
        if not self.check_seq():
            print "thisin isn't sequencable"
            return
        #######################################
        if not self.index:
            print "at start"
            return {}
        else:
            return self.__getitem__(self.index -1)
   
    def rewind(self, msg_num):    
        if not self.check_seq():
            print "thisin isn't sequencable"
            return
        #######################################
        return self.goto(msg_num)

    def forward(self, msg_num):
        if not self.check_seq():
            print "thisin isn't sequencable"
            return
        #######################################
        return self.goto(msg_num)
        
    def __str__(self):
        return "my index = {0}, and value = {1}".format(self.index, self.cache)
    
    def __repr__(self):
        return repr((self.index, self.cache))

    def __iter__(self):
        if not self.check_seq():
            print "thisin isn't sequencable"
            return
        #######################################
        "to see all the messages from 0"
        if self.index == None:
            fields_to_apply = self.msg_sequence
        else:
            fields_to_apply = get_list_slice_reg(self.msg_sequence, high_val = self.index+1)
        for each_update in fields_to_apply:
            self.update(each_update)
            self.index += 1
            yield self.cache


#------------------------------------------
# Graph stuff
#-----------------------------------------

#import matplotlib.pyplot as plt
import numpy as np

def add_plot():
    plt.hold(True)
    
def open_plot():
    plt.hold(False)
    
def plot_inter():
    plt.interactive(True)
    
def plot(*set_of_lists, **kargs):
    plt.plot(*set_of_lists, **kargs)

def save_plot(fig_name=None):
    if fig_name==None:
        plt.savefig('My_figure')
    else:
        plt.savefig(fig_name)
