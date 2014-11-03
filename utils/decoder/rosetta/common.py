from collections import OrderedDict
try:
    import configparser
except:
    import ConfigParser as configparser
import struct
import functools
import gzip

def open_file(file_name, mode='rb'):
    if file_name.endswith('.gz'):
        if gzip:
            return gzip.open(file_name, mode)
        else:
            raise NotImplementedError("python must have zlib problems, couldn't import gzip, need to rebuild this python and fix zlib problems.")
    else:
        return open(file_name, mode)

def get_spec(inipath):
    spec = OrderedDict()
    x = configparser.RawConfigParser(allow_no_value=True)
    x.optionxform = str
    x.read(inipath)
    endian = x.get('GLOBAL', '_endian')
    for section in x.sections():
        
        if section != 'GLOBAL':
            spec[section] = {}
            spec[section]['spec'] = x.items(section)
            spec[section]['flds'] = [f for f,_ in spec[section]['spec']]
            spec[section]['iformat'] =  [(endian + y, struct.calcsize(y)) for _, y in spec[section]['spec']]
            spec[section]['format'] = endian + ''.join(y for _, y in spec[section]['spec'])
            spec[section]['size'] = struct.calcsize(spec[section]['format'])
        else:
            spec['endian'] = endian

    return spec

#---
# decoder factory
#---
def decode(flds, iformat, format, size, data, index=0):
    msg = OrderedDict()
    for fld, form_size in zip(flds, iformat):
        element_format, element_size = form_size
        try:
            val = struct.unpack_from(element_format, data, index)[0]
        except struct.error:
            import pdb
            pdb.set_trace()
        index += element_size
        msg[fld] = val
    return msg, index


def build_standard_msg_parser(msg_spec):

    return functools.partial(decode, msg_spec['flds'], msg_spec['iformat'], msg_spec['format'], msg_spec['size'])

def pb_parser(file_name):
    finished_message = True
    data = b''
    remaining = 0
    
    meta_start = b"*M*E*T*A*S*T*A*R*T*"
    len_meta_start = len(meta_start)
    meta_end = b"*M*E*T*A*E*N*D*"
    len_meta_end = len(meta_end)
    
    with open_file(file_name,'rb') as file_obj:
        for linenum, line in enumerate(file_obj):

            end_index = line.find(meta_end) 

            if end_index != -1:

                meta_header_end_index = end_index + len_meta_end
                meta_header = line[:meta_header_end_index]
                data = line[meta_header_end_index:]
                datalength = len(data)-1

                meta = meta_header[len_meta_start:-len_meta_end].split(b'/')
                length = int(meta[2])
                
                if datalength >= length:
                    data = data[0:length]
                    finished_message = True
                else:
                    remaining = length-datalength
                    finished_message = False
            
            elif finished_message is False:
                datalength = len(line)
                if remaining > datalength: 
                    data = b''.join([data, line])
                    remaining = remaining-datalength
                else:
                    data = b''.join([data, line[0:remaining]])
                    finished_message = True
                    remaining = 0
            
            if finished_message is True:
                yield data
                data = b''
                finished_message = False
