import sys
import struct
import pprint

from rosetta.common import get_spec, decode, build_standard_msg_parser, pb_parser

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
            msg, index = sec_2(data, index)
            submessages.append(msg)
        return msg, index
    decoders[11] = decoder_11

    liq_1 = build_standard_msg_parser(spec['13.liquidity_provider'])
    liq_2 = build_standard_msg_parser(spec['13.sub.liquidity_provider'])
    def decoder_13(data, index):
        msg, index = liq_1(data, index)
        submessages = []
        msg['submessages'] = submessages
        for _ in range(msg['NoLiquidityProviders']):
            msg, index = liq_2(data, index)
            submessages.append(msg)
        return msg, index
    decoders[13] = decoder_13

    agg_1 = build_standard_msg_parser(spec['53.aggregate_order_book_update'])
    agg_2 = build_standard_msg_parser(spec['53.sub.aggregate_order_book_update_spec2'])
    def decoder_53(data, index):
        msg, index = agg_1(data, index)
        submessages = []
        msg['submessages'] = submessages
        for _ in range(msg['NoEntries']):
            msg, index = agg_2(data, index)
            submessages.append(msg)
        return msg, index
    decoders[53] = decoder_53

    bro_1 = build_standard_msg_parser(spec['54.broker_queue'])
    bro_2 = build_standard_msg_parser(spec['54.sub.broker_queue'])
    def decoder_54(data, index):
        msg, index = bro_1(data, index)
        submessages = []
        msg['submessages'] = submessages
        for _ in range(msg['ItemCount']):
            msg, index = bro_2(data, index)
            submessages.append(msg)
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
            msg, index = news2(data, index)
            market_codes.append(msg)
        n_msg, index = news3(data, index)
        n_msg, index = news4(data, index)
        sec_codes = []
        msg['sec_codes'] = sec_codes
        for _ in range(n_msg['NoSecurityCodes']):
            msg, index = news5(data, index)
            sec_codes.append(msg)
        n_msg, index = news6(data, index)
        n_msg, index = news7(data, index)
        news_lines = []
        msg['news_lines'] = news_lines
        for _ in range(n_msg['NoNewsLines']):
            msg, index = news8(data, index)
            news_lines.append(msg)
        return msg, index
    decoders[22] = decoder_22

    return decoders

def decode_playback(decoders, spec, playback):
    header_decoder = decoders['header']
    endian = spec['endian']
    header_format = spec['header']['format']
    header_size = struct.calcsize(header_format)
    msg_num = 0

    for data in pb_parser(playback):
        if not data:
            break
        msg_num += 1

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
        yield msg

    print(msg_num)

def main():
    spec = get_spec('hkseomd.ini')
    decoders = build_msg_parsers(spec)
    for msg in decode_playback(decoders, spec, sys.argv[1]):
        #pass
        pprint.pprint(msg)

if __name__ == '__main__':
    main()
