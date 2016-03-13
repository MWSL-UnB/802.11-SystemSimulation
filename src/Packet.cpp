/*
* Copyright (c) 2002-2015 by Microwave and Wireless Systems Laboratory, by Andre Barreto and Calil Queiroz
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include "Packet.h"
#include "myexception.h"

#include <iostream>
#include <iomanip>


#include "Terminal.h"

//////////////////////
// OFDM symbol period
const double symbol_period = 4e-6;

/////////////////////////////////////////////////////////////////////
// MAC overhead for different packet types (including FCS) in bytes
const unsigned data_packet_overhead = 28;
const unsigned ack_packet_overhead  = 14;
const unsigned rts_packet_overhead  = 20;
const unsigned cts_packet_overhead  = 14;

// Define these correctly
const unsigned addba_rqst_overhead  = 14;
const unsigned addba_rsps_overhead  = 14;
const unsigned bar_packet_overhead  = 20;
const unsigned ba_packet_overhead   = 20;

/////////////////////////////
// physical layer overhead 
const unsigned service_field_overhead = 2; // service field (in OFDM symbols)
const unsigned phy_overhead = 5; // preambles + SIGNAL field (in OFDM symbols)
const unsigned coding_overhead = 6; // number of termination bits

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// enum transmission_mode                                                     //
////////////////////////////////////////////////////////////////////////////////

///////////////////////
// tx_mode_to_double //
///////////////////////
double tx_mode_to_double (transmission_mode tm) {
  switch(tm) {
    case M6 : return  6.0;
    case M9 : return  9.0;
    case M12: return 12.0;
    case M18: return 18.0;
    case M24: return 24.0;
    case M36: return 36.0;
    case M48: return 48.0;
    case M54: return 54.0;
    default : return    0;
  }
}

////////////////////////
// output operator << //
////////////////////////
ostream& operator<< (ostream& os, const transmission_mode& tm) {
  unsigned w = os.width() - 5;
  switch(tm) {
    case OPT: return os << "OPT";
    case SUBOPT: return os << "SUBOPT";
    case M0 : return os << setw(w) << 0 << " Mbps";
    case M6 : return os << setw(w) <<  6 << " Mbps";
    case M9 : return os << setw(w) <<  9 << " Mbps";
    case M12: return os << setw(w) << 12 << " Mbps";
    case M18: return os << setw(w) << 18 << " Mbps";
    case M24: return os << setw(w) << 24 << " Mbps";
    case M36: return os << setw(w) << 36 << " Mbps";
    case M48: return os << setw(w) << 48 << " Mbps";
    case M54: return os << setw(w) << 54 << " Mbps";
  }
}

///////////////////////
// input operator >> //
///////////////////////
istream& operator>> (istream& is, transmission_mode& tm) {
  string str;
  is >> str;

  if (str == "OPT") tm = OPT;
  else if (str == "SUBOPT") tm = SUBOPT;
  else if (str == "M6") tm = M6;
  else if (str == "M9") tm = M9;
  else if (str == "M12") tm = M12;
  else if (str == "M18") tm = M18;
  else if (str == "M24") tm = M24;
  else if (str == "M36") tm = M36;
  else if (str == "M48") tm = M48;
  else if (str == "M54") tm = M54;
  else is.clear(ios::failbit);

  return is;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// calc_duration                                                              //
//
// calculates the packet duration                                             //
////////////////////////////////////////////////////////////////////////////////
timestamp calc_duration (unsigned nbits, transmission_mode mode) {

  unsigned bits_per_symbol;

  switch (mode) {
    case M6  : bits_per_symbol =  24; break;
    case M9  : bits_per_symbol =  36; break;
    case M12 : bits_per_symbol =  48; break;
    case M18 : bits_per_symbol =  72; break;
    case M24 : bits_per_symbol =  96; break;
    case M36 : bits_per_symbol = 144; break;
    case M48 : bits_per_symbol = 192; break;
    case M54 : bits_per_symbol = 216; break;
    case M0  :
      return timestamp(0);
    default  :
      throw(my_exception(GENERAL,
            "Transmission mode not supported in packet constructor"));
  }

  // calculate number of OFDM symbols in payload
  // consider termination bits
  unsigned nsymbols = (nbits + coding_overhead)/ bits_per_symbol;
  if ((nbits+coding_overhead)%bits_per_symbol) nsymbols++;

  // add preambles and SIGNAL field
  nsymbols += phy_overhead;

  // calculate time
  return timestamp(double(nsymbols) * symbol_period);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Packet                                                               //
////////////////////////////////////////////////////////////////////////////////

long_integer Packet::packet_count = 0;

////////////////////////////////////////////////////////////////////////////////
// MSDU Constructor                                                           //
////////////////////////////////////////////////////////////////////////////////
MSDU::MSDU(unsigned n, Terminal* from , Terminal* to, unsigned priority,
           timestamp gen_time)
           : nbytes_data(n), tid(priority), time_created(gen_time) {

  source = from;
  target = to;
}

////////////////////////////////////////////////////////////////////////////////
// MPDU Constructor                                                           //
////////////////////////////////////////////////////////////////////////////////
MPDU::MPDU(packet_type tp, Terminal* from, Terminal* to, double p,
           transmission_mode r, timestamp nav)
           : t(tp), mode(r), tx_power(p), net_all_vec(nav) {

	pcks2ACK.clear();

	source = from;
	target = to;

	switch (t) {
	case ACK :
		nbytes_overhead = service_field_overhead + ack_packet_overhead;
		break;
	case RTS :
		nbytes_overhead = service_field_overhead + rts_packet_overhead;
		break;
	case CTS :
		nbytes_overhead = service_field_overhead + cts_packet_overhead;
		break;
	case ADDBArqst :
		nbytes_overhead = service_field_overhead + addba_rqst_overhead;
		break;
	case ADDBArsps :
		nbytes_overhead = service_field_overhead + addba_rsps_overhead;
		break;
	case BAR :
		nbytes_overhead = service_field_overhead + bar_packet_overhead;
		break;
	case BA :
		nbytes_overhead = service_field_overhead + ba_packet_overhead;
		break;
	case DUMMY :
		return;
	default :
		throw(my_exception(GENERAL,
				"Packet type not supported in MPDU constructor"));
  }

  nbits = nbytes_overhead*8;
  packet_duration = calc_duration (nbits, mode);
}

void MPDU::set_pcks2ACK(const vector<long_integer>& p2a) {

	  switch(t) {
	  case BAR: break;
	  case BA : break;
	  default: throw(my_exception(GENERAL,
				"Attempt to set_pcks2ACK on non BA packet."));
	  }
	  pcks2ACK = p2a;
  }

////////////////////////////////////////////////////////////////////////////////
// DataMPDU Constructors                                                      //
////////////////////////////////////////////////////////////////////////////////
DataMPDU::DataMPDU (unsigned n, Terminal* from, Terminal* to, double p, 
                    transmission_mode r, timestamp nav, unsigned priority,
                    unsigned frag, unsigned nfrags,unsigned mid) 
                    : nbytes_data(n), tid(priority), frag_number(frag),
                      frag_total(nfrags), msdu_id(mid){

  t = DATA;
  mode = r;
  source = from;
  target = to;
  tx_power = p;
  net_all_vec = nav;
  
  nbytes_overhead = service_field_overhead + data_packet_overhead;
  nbits = (nbytes_data + nbytes_overhead)*8;
  packet_duration = calc_duration (nbits, mode);
}

////////////////////////////////////////////////////////////////////////////////
DataMPDU::DataMPDU (MSDU pck, int n, unsigned frag, unsigned nfrags, double p,
                    transmission_mode r, timestamp nav)
                    : frag_number(frag), frag_total(nfrags) {

  t = DATA;
  nbytes_data = (n >= 0)? n : pck.get_nbytes();
  mode = r;
  source = pck.get_source();
  target = pck.get_target();
  tid = pck.get_tid();
  msdu_id = pck.get_id();
  tx_power = p;
  net_all_vec = nav;
  
  nbytes_overhead = service_field_overhead + data_packet_overhead;
  nbits = (nbytes_data + nbytes_overhead)*8;
  packet_duration = calc_duration (nbits, mode);

}

////////////////////////////////////////////////////////////////////////////////
// MPDU output operator <<                                                    //
////////////////////////////////////////////////////////////////////////////////
ostream& operator << (ostream& os, const MPDU& p) {
  switch (p.t) {
    case DATA:
      return os << "data packet " << p.id << " from " << *(p.source) << " to " 
                << *(p.target);
    case ACK:
      return os << "ACK packet " << p.id << " from " << *(p.source) << " to "
                << *(p.target);
    case CTS:
      return os << "CTS packet " << p.id << " from " << *(p.source) << " to "
                << *(p.target);
    case RTS:
      return os << "RTS packet " << p.id << " from " << *(p.source) << " to "
                << *(p.target);
    case ADDBArqst:
          return os << "ADDBA request packet " << p.id << " from " << *(p.source) << " to "
                    << *(p.target);
    case ADDBArsps:
    	return os << "ADDBA response packet " << p.id << " from " << *(p.source) << " to "
    			<< *(p.target);
    case BAR:
    	return os << "BAR packet " << p.id << " from " << *(p.source) << " to "
    			<< *(p.target);
    case BA:
    	return os << "BA packet " << p.id << " from " << *(p.source) << " to "
    			<< *(p.target);
  }
}


