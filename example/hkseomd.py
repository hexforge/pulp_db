import sys
import struct
import pprint

from collections import OrderedDict
try:
    import configparser
except ImportError:
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
                raw = meta_header + data
                yield raw, data
                data = b''
                finished_message = False


def build_msg_parsers(spec):
    decoders = {}
    
    endian = spec['endian']

    decoders['header'] = build_standard_msg_parser(spec['header'])
    decoders[100] = build_standard_msg_parser(spec['100.sequence_reset'])
    decoders[101] = build_standard_msg_parser(spec['101.logon'])
    decoders[102] = build_standard_msg_parser(spec['102.logon_responce'])
    decoders[201] = build_standard_msg_parser(spec['201.retransmission_request'])
    decoders[202] = build_standard_msg_parser(spec['202.retransmission_responce'])
    decoders[203] = build_standard_msg_parser(spec['203.refresh_complete'])
    decoders[10] = build_standard_msg_parser(spec['10.market_definition'])
    decoders[14] = build_standard_msg_parser(spec['14.currency_rate'])
    decoders[20] = build_standard_msg_parser(spec['20.trading_session_status'])
    decoders[21] = build_standard_msg_parser(spec['21.security_status'])
    decoders[30] = build_standard_msg_parser(spec['30.add_order'])
    decoders[31] = build_standard_msg_parser(spec['31.modify_order'])
    decoders[32] = build_standard_msg_parser(spec['32.delete_order'])
    decoders[33] = build_standard_msg_parser(spec['33.add_odd_lot_order'])
    decoders[34] = build_standard_msg_parser(spec['34.delete_odd_lot_order'])
    decoders[51] = build_standard_msg_parser(spec['51.trade_cancel'])
    decoders[52] = build_standard_msg_parser(spec['52.trade_ticker'])
    decoders[62] = build_standard_msg_parser(spec['62.closing_price'])
    decoders[40] = build_standard_msg_parser(spec['40.nominal_price'])
    decoders[41] = build_standard_msg_parser(spec['41.indicative_equilibrium_price'])
    decoders[60] = build_standard_msg_parser(spec['60.statistics'])
    decoders[61] = build_standard_msg_parser(spec['61.market_turnover'])
    decoders[44] = build_standard_msg_parser(spec['44.yield'])
    decoders[70] = build_standard_msg_parser(spec['70.index_definition'])
    decoders[71] = build_standard_msg_parser(spec['71.index_data'])
    decoders[55] = build_standard_msg_parser(spec['55.top_of_book'])
    decoders[42] = build_standard_msg_parser(spec['42.estimated_average_settlement_price'])
    decoders[50] = build_standard_msg_parser(spec['50.Trade'])

    sec_1 = build_standard_msg_parser(spec['11.security_definition'])
    sec_2 = build_standard_msg_parser(spec['11.sub.security_definition'])
    def decoder_11(data, index):
        msg, index = sec_1(data, index)
        submessages = []
        msg['submessages'] = submessages
        for _ in range(msg['NoUnderlyingSecurities']):
            sub_msg, index = sec_2(data, index)
            submessages.append(sub_msg)
        return msg, index
    decoders[11] = decoder_11

    liq_1 = build_standard_msg_parser(spec['13.liquidity_provider'])
    liq_2 = build_standard_msg_parser(spec['13.sub.liquidity_provider'])
    def decoder_13(data, index):
        msg, index = liq_1(data, index)
        submessages = []
        msg['submessages'] = submessages
        for _ in range(msg['NoLiquidityProviders']):
            sub_msg, index = liq_2(data, index)
            submessages.append(sub_msg)
        return msg, index
    decoders[13] = decoder_13

    agg_1 = build_standard_msg_parser(spec['53.aggregate_order_book_update'])
    agg_2 = build_standard_msg_parser(spec['53.sub.aggregate_order_book_update_spec2'])
    def decoder_53(data, index):
        msg, index = agg_1(data, index)
        submessages = []
        msg['submessages'] = submessages
        for _ in range(msg['NoEntries']):
            sub_msg, index = agg_2(data, index)
            submessages.append(sub_msg)
        return msg, index
    decoders[53] = decoder_53

    bro_1 = build_standard_msg_parser(spec['54.broker_queue'])
    bro_2 = build_standard_msg_parser(spec['54.sub.broker_queue'])
    def decoder_54(data, index):
        msg, index = bro_1(data, index)
        submessages = []
        msg['submessages'] = submessages
        for _ in range(msg['ItemCount']):
            sub_msg, index = bro_2(data, index)
            submessages.append(sub_msg)
        return msg, index
    decoders[54] = decoder_54

    news = build_standard_msg_parser(spec['22.news'])
    news1 = build_standard_msg_parser(spec['22.sub1.news'])
    news2 = build_standard_msg_parser(spec['22.sub2.news'])
    news3 = build_standard_msg_parser(spec['22.sub3.news'])
    news4 = build_standard_msg_parser(spec['22.sub4.news'])
    news5 = build_standard_msg_parser(spec['22.sub5.news'])
    news6 = build_standard_msg_parser(spec['22.sub6.news'])
    news7 = build_standard_msg_parser(spec['22.sub7.news'])
    news8 = build_standard_msg_parser(spec['22.sub8.news'])

    def decoder_22(data, index):
        msg, index = news(data, index)
        n_msg, index = news1(data, index)
        msg.update(n_msg)

        market_codes = []
        msg['market_codes'] = market_codes
        for _ in range(n_msg['NoMarketCodes']):
            sub_msg, index = news2(data, index)
            market_codes.append(sub_msg)
        n_msg, index = news3(data, index)
        n_msg, index = news4(data, index)
        sec_codes = []
        msg['sec_codes'] = sec_codes
        for _ in range(n_msg['NoSecurityCodes']):
            sub_msg, index = news5(data, index)
            sec_codes.append(sub_msg)
        n_msg, index = news6(data, index)
        n_msg, index = news7(data, index)
        news_lines = []
        msg['news_lines'] = news_lines
        for _ in range(n_msg['NoNewsLines']):
            sub_msg, index = news8(data, index)
            news_lines.append(sub_msg)
        return msg, index
    decoders[22] = decoder_22

    return decoders

def pb_msg_decoder(bdata):
    meta_start = b"*M*E*T*A*S*T*A*R*T*"
    meta_end = b"*M*E*T*A*E*N*D*"
    meta, payload = bdata.split(meta_end, 1)
    h, hint, len, ip, port, time_str, time_num, thing, thing = meta.split(b'/')
    data = {"payload": payload,
            "meta": meta,
            "hint": hint,
            "ip": ip,
            "time_str": time_str,
            "time_num": time_num}
    return data

def decode_playback(decoders, spec, playback):
    decoder = make_decoder(decoders, spec)
    msg_num = 0
    for raw, data in pb_parser(playback):
        if not data:
            break
        msg_num += 1

        msg = decoder(data)
        yield msg, raw

    print("Decoded x messages", msg_num)

def make_decoder(decoders, spec):
    header_decoder = decoders['header']
    endian = spec['endian']
    header_format = spec['header']['format']
    header_size = struct.calcsize(header_format)

    def decode_message(data):
        msg = []
        
        # HEADER
        index = 0 
        try:
            decoded_header, index = header_decoder(data, index)
        except struct.error:
            raise
        msg.append(decoded_header)

        # SUBMSGS
        number_of_submsgs = decoded_header['MsgCount']
        for _ in range(number_of_submsgs):
            try:
                size = struct.unpack(endian+'H', data[index:index+2])[0]
                typ = struct.unpack(endian+'H', data[index+2:index+4])[0]
                sub_msg, index = decoders[typ](data, index)
            except (struct.error, KeyError):
                import pdb
                pdb.set_trace()
                raise
            msg.append(sub_msg)
        return msg
    return decode_message

def main():
    spec = get_spec('hkseomd.ini')
    decoders = build_msg_parsers(spec)
    for msg, raw in decode_playback(decoders, spec, sys.argv[1]):
        #pass
        pprint.pprint(msg)

if __name__ == '__main__':
    main()
