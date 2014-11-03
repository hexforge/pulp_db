import sys

def pb_parser(file_name):
    finished_message = True
    data = b''
    remaining = 0
    
    meta_start = b"*M*E*T*A*S*T*A*R*T*"
    len_meta_start = len(meta_start)
    meta_end = b"*M*E*T*A*E*N*D*"
    len_meta_end = len(meta_end)
    
    with open(file_name,'rb') as file_obj:
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
                #print meta, data
                #yield meta, data
                data = b''
                finished_message = False

pb_parser(sys.argv[1])

