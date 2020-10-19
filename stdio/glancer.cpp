#include "mwx_stdio.h"

serparser_local<256> parser_ser;
void main_loop();

/*** the setup procedure (called on boot) */
void setup() {
    // start console
    Serial.begin();

    // init ser parser (heap alloc)
    parser_ser.begin(PARSER::ASCII);
}

/*** the loop procedure (called every event) */
void loop() {
    main_loop();
}

// displays whole payload as hex
template <typename T>
void print_unknown(T& payl) {
	Serial << ":MSG=0x";
	for(auto x : payl) {
        Serial << format("%02x", x);
	}
}

// TWELITE PAL information
void print_pal(pktparser& pkt) {
	auto&& pal = pkt.use<TwePacketPal>();

	if (pal.is_PalEvent()) {
		Serial << "PAL_EVENT:ID=" << int(pal.get_PalEvent().u8event_id);
	} else switch(pal.u8palpcb) {
		case E_PAL_PCB::MAG:
		{
			// generate pal board specific data structure.
			PalMag mag = pal.get_PalMag();

			Serial << "PAL_MAG";
			Serial << ":STAT=" << int(mag.u8MagStat);
			Serial << "(";
			switch(mag.u8MagStat & 0x7F) {
			case 0: Serial << "NO MANGET"; break;
			case 1: Serial << "N POLE"; break;
			case 2: Serial << "S POLE"; break;
			}
			Serial << ")";
		} break;

		case E_PAL_PCB::AMB:
		{
			// generate pal board specific data structure.
			PalAmb amb = pal.get_PalAmb();

			Serial << "PAL_AMB";
			Serial << ":TEMP=" << (double)amb.i16Temp / 100.0;
			Serial << ":HUMD=" << int((amb.u16Humd + 50) / 100);
			Serial << ":LUMI=" << int(amb.u32Lumi);
		} break;

		case E_PAL_PCB::MOT:
		{
			// generate pal board specific data structure.
			PalMot mot = pal.get_PalMot();

			Serial << "PAL_MOT";
			Serial << ":SAMPLES=" << int(mot.u8samples);
			Serial << ":SR=" << int(mot.u8sample_rate_code);
			for (int i = 0; i < mot.u8samples; i++) {
				if (mot.u32StoredMask & (1UL << (i + 1))) { // check if stored properly.
					Serial << ":" << i << "(" << mot.i16X[i];
					Serial << "," << mot.i16Y[i];
					Serial << "," << mot.i16Z[i];
					Serial << ")";
				}
			}
		} break;

		default:
			;
	}
}

// App_UART
void print_app_uart(pktparser& pkt) {
	auto&& aur = pkt.use<TwePacketAppUART>();

	Serial << "App_UART";
	auto p = aur.payload.begin();
	
	// display the rest as hex.
	Serial << "MSG=0x";
	while(p != aur.payload.end()) {
        Serial << format("%02x", *p);
		++p;
	}
}

// Act samples
void print_act(pktparser& pkt) {
	auto&& act = pkt.use<TwePacketActStd>();

	if (act.payload.size() >= 5) { // should have 5 bytes or more (FOURCC, DATA ...)
		auto p = act.payload.begin();
		// display four cc
		Serial << "Act[";
		for (int i = 0; i < 4; ++i, ++p) Serial << char_t(*p);
		Serial << "]=0x";

		// display the rest as hex.
		while(p != act.payload.end()) {
            Serial << format("%02x", *p);
			++p;
		}
	}
}

// App_Twelite 0x81
void print_app_twelite(pktparser& pkt) {
	auto&& atw = pkt.use<TwePacketTwelite>();

	Serial << "App_Twelite";
	Serial << ":DI1..4=" << char(atw.DI1 ? 'L' : 'H') << char(atw.DI2 ? 'L' : 'H') 
                         << char(atw.DI3 ? 'L' : 'H') << char(atw.DI4 ? 'L' : 'H');
	Serial << ":AI1..4=(";
    uint16_t adc_vals[4] = {
        atw.u16Adc1, atw.u16Adc2, atw.u16Adc3, atw.u16Adc4
    };
    for (int i = 0; i < 4; i++) {
        if(i > 0) Serial << ',';
        if (atw.Adc_active_mask & (1 << i)) {
            Serial << format("%04d", adc_vals[i]);
        } else Serial << "----";
    }
	Serial << ')';
}

// App_IO
void print_app_io(pktparser& pkt) {
	auto&& aio = pkt.use<TwePacketAppIO>();

	Serial << "App_IO";
	Serial << ":DI(1..12)=";
	for (int i = 0; i < 12; i++) {
        if (aio.DI_active_mask & (1 << i))
		    Serial << char_t((aio.DI_mask & (1 << i)) ? 'L' : 'H');
        else
            Serial << '-';
    }
	
	Serial << ":INT=";
	for (int i = 0; i < 12; i++) {
		Serial << char((aio.DI_int_mask & (1 << i)) ? '!' : '-');
	}
}

// App_TAG (no detail information about an each sensor.)
void print_app_tag(pktparser& pkt) {
	auto&& tag = pkt.use<TwePacketAppTAG>();

	Serial << "App_TAG";
	Serial << ":SENSOR=" << int(tag.u8sns);
	
	Serial << ":DATA=0x";
	for (auto x : tag.payload) {
        Serial << format("%02x", x);
	}
}

// the loop
void main_loop() {
    int c;
	while ((c = Serial.read()) >= 0) {
		if (c == 0x03 || c == 0x04) {
			mwx_exit(0); // exit by Ctrl+C or Ctrl+D code
			return;
		}

		if (c == '\n') c = '\r'; // convert \n to \r (\r is required as the end of message)
        
		parser_ser.parse(c);

		if (parser_ser.available()) {
            Serial << parser_ser;

			Serial << "PKT";
			auto&& payl = parser_ser.get_buf(); // payload data array
			auto&& typ = identify_packet_type(payl.begin(), payl.end());  // packet type
			Serial << ":Typ=" << int(typ);

            pktparser pkt;

            // parse data according to packet type
			switch (typ) {
			case E_PKT::PKT_PAL: typ = pkt.parse<TwePacketPal>(payl.begin(), payl.end()); break;
			case E_PKT::PKT_ACT_STD: typ = pkt.parse<TwePacketActStd>(payl.begin(), payl.end()); break;
			case E_PKT::PKT_TWELITE: typ = pkt.parse<TwePacketTwelite>(payl.begin(), payl.end()); break;
			case E_PKT::PKT_APPIO: typ = pkt.parse<TwePacketAppIO>(payl.begin(), payl.end()); break;
			case E_PKT::PKT_APPUART: typ = pkt.parse<TwePacketAppUART>(payl.begin(), payl.end()); break;
			case E_PKT::PKT_APPTAG: typ = pkt.parse<TwePacketAppTAG>(payl.begin(), payl.end()); break;
			default:
                ; // e.g. UART message of App_Twelite, App_UART(simple format), or corrupted.
			}

            // display common information
			if (typ != E_PKT::PKT_ERROR) {
                // query common packet type.
                auto&& pkt_common = pkt.use<TwePacket>();

				Serial << ":"
					<< "Lq="    << format("%02d", pkt_common.common.lqi)
					<< ":Ad=0x" << format("%08x", pkt_common.common.src_addr)
					<< "(0x"    << format("%02x)", pkt_common.common.src_lid)
					<< ":Vmv="  << format("%04d", pkt_common.common.volt)
					<< ":Tms="  << format("%04d", pkt_common.common.tick % 8192)
					<< ":"
                    << crlf << "  ";
			}

            // display each object.
            switch (typ) {
			case E_PKT::PKT_PAL: print_pal(pkt); break;
			case E_PKT::PKT_ACT_STD: print_act(pkt); break;
			case E_PKT::PKT_TWELITE: print_app_twelite(pkt); break;
			case E_PKT::PKT_APPIO: print_app_io(pkt); break;
			case E_PKT::PKT_APPUART: print_app_uart(pkt); break;
			case E_PKT::PKT_APPTAG: print_app_tag(pkt); break;
			default: print_unknown(payl); // e.g. UART message of App_Twelite, App_UART(simple format), or corrupted.
			}

			Serial << crlf;
		}
	}
}
