#include "mwx_stdio.h"

serparser_local<256> parser_ser;

/*** the setup procedure (called on boot) */
void setup() {
    // init ser parser (heap alloc)
    parser_ser.begin(PARSER::ASCII);
}

void loop() {
    int c;
    while((c = getchar()) != -1) { // read from stdin
        if (c == '\n') c = '\r'; // convert \n to \r (\r is required as the end of message)
        parser_ser.parse(c); // ASCII format parser (e.g. :001122X)
        if (parser_ser.available()) { // finishes ASCII format (now get { 0x00, x011, 0x22 } ) 
			auto&& payl = parser_ser.get_buf(); // payload data array
			auto&& typ = identify_packet_type(payl.begin(), payl.end());  // packet type

            bool b_others = false;
            if (typ == E_PKT::PKT_TWELITE) { // identfied packet is TWELITE 0x81
                pktparser pkt;
                typ = pkt.parse<TwePacketTwelite>(payl.begin(), payl.end()); // parse it!
                if (typ == E_PKT::PKT_TWELITE) { // parse error check
                    auto&& app = pkt.use<TwePacketTwelite>(); // get application object which contains app specific data.
                    printf("TWELITE: SRC=%08X LQI=%03d DI=%1X/%1X\r\n"
                        , app.u32addr_src
                        , app.u8lqi
                        , app.DI_mask & 0xF
                        , app.DI_active_mask & 0xF
                    );
                } else b_others = true;
            } else b_others = true;

            if (b_others) printf("OTHER: TYP=%d\r\n", int(typ));
        }
    }
    mwx_exit(0);
}