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

#include "Standard.h"
#include "Terminal.h"

//////////////////////
// OFDM symbol period
//const double symbol_period = 4e-6;

/////////////////////////////////////////////////////////////////////
// MAC overhead for different packet types (including FCS) in bytes
const unsigned data_packet_overhead = 28;
const unsigned ack_packet_overhead  = 14;
const unsigned rts_packet_overhead  = 20;
const unsigned cts_packet_overhead  = 14;
const unsigned ba_packet_overhead  = 20;
const unsigned mpdu_delimiter_overhead = 4;

/////////////////////////////
// physical layer overhead 
const unsigned service_field_overhead = 2; // service field (in OFDM symbols)
const unsigned phy_overhead = 5; // preambles + SIGNAL field (in OFDM symbols)
const unsigned coding_overhead = 6; // number of termination bits

////////////////////////////////////////////////////////////////////////////////
// enum transmission_mode                                                     //
////////////////////////////////////////////////////////////////////////////////

////////////////////////
// output operator << //
////////////////////////
ostream& operator<< (ostream& os, const transmission_mode& tm) {
  unsigned w = os.width() - 5;
  switch(tm) {
    case OPT: return os << "OPT";
    case SUBOPT: return os << "SUBOPT";
    default: return os << setw(w) << Standard::tx_mode_to_double(tm) << " Mbps";
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
  else if (str == "MCS0") tm = MCS0;
  else if (str == "MCS1") tm = MCS1;
  else if (str == "MCS2") tm = MCS2;
  else if (str == "MCS3") tm = MCS3;
  else if (str == "MCS4") tm = MCS4;
  else if (str == "MCS5") tm = MCS5;
  else if (str == "MCS6") tm = MCS6;
  else if (str == "MCS7") tm = MCS7;
  else if (str == "MCS8") tm = MCS8;
  else if (str == "MCS9") tm = MCS9;
  else is.clear(ios::failbit);

  return is;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// calc_duration                                                              //
//
// calculates the packet duration                                             //
////////////////////////////////////////////////////////////////////////////////
timestamp calc_duration (unsigned nbits, transmission_mode mode, bool addPre) {

  unsigned bits_per_symbol;

  if(mode == MCS || mode > Standard::get_maxMCS()){
	  cout << "In the if" << endl;
	  cout << "Standard: " << Standard::get_standard() << endl;
	  return timestamp(0);
  }
  bits_per_symbol = Standard::txMode_bits_per_symbol(mode);

  // calculate number of OFDM symbols in payload
  // consider termination bits
  unsigned nsymbols = (nbits + coding_overhead)/ bits_per_symbol;
  if ((nbits+coding_overhead)%bits_per_symbol) nsymbols++;

  // add preambles and SIGNAL field
  if(addPre) nsymbols += phy_overhead;

  // calculate time
  return timestamp(double(nsymbols) * Standard::get_symbol_period());
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

  retry_count = 0;
  source = from;
  target = to;
}

////////////////////////////////////////////////////////////////////////////////
// MPDU Constructor                                                           //
////////////////////////////////////////////////////////////////////////////////
MPDU::MPDU(packet_type tp, Terminal* from, Terminal* to, double p,
           transmission_mode r, timestamp nav)
           :  mode(r), t(tp), tx_power(p), net_all_vec(nav) {

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
    case BA :
      nbytes_overhead = service_field_overhead + ba_packet_overhead;
      break;
    case DUMMY :
      return;
    default :
      throw(my_exception(GENERAL,
            "Packet type not supported in MPDU constructor"));
  }

  ACKpol = noACK;

  nbits = nbytes_overhead*8;
  packet_duration = calc_duration (nbits, mode, true);

  pcks2ACK.clear();
}

////////////////////////////////////////////////////////////////////////////////
// MPDU setPcks2Ack                                                      //
////////////////////////////////////////////////////////////////////////////////
void MPDU::setPcks2Ack(const vector<long_integer>& pcks2Ack) {
	switch(t) {
	case BA:
		break;
	default:
		throw(my_exception(GENERAL,
		            "Attempt to initialize pcks2ACK for non-BA packet"));
	}
	pcks2ACK = pcks2Ack;
}

////////////////////////////////////////////////////////////////////////////////
// DataMPDU Constructors                                                      //
////////////////////////////////////////////////////////////////////////////////
DataMPDU::DataMPDU (unsigned n, Terminal* from, Terminal* to, double p, 
                    transmission_mode r, timestamp nav, unsigned priority,
                    unsigned frag, unsigned nfrags,unsigned mid, ACKpolicy apol,
					bool addP)
                    : tid(priority), frag_number(frag),
                      frag_total(nfrags), msdu_id(mid), nbytes_data(n){

  t = DATA;
  mode = r;
  source = from;
  target = to;
  tx_power = p;
  net_all_vec = nav;
  ACKpol = apol;
  
  nbytes_overhead = service_field_overhead + data_packet_overhead;
  if(ACKpol == blockACK) nbytes_overhead += mpdu_delimiter_overhead;

  if(ACKpol == normalACK && !addP) throw(my_exception(GENERAL,
          "Normal ACK packet without preamble."));

  nbits = (nbytes_data + nbytes_overhead)*8;
  packet_duration = calc_duration (nbits, mode, addP);
}

////////////////////////////////////////////////////////////////////////////////
DataMPDU::DataMPDU (MSDU pck, int n, unsigned frag, unsigned nfrags, double p,
                    transmission_mode r, timestamp nav, ACKpolicy apol, bool addP)
                    : frag_number(frag), frag_total(nfrags){

  t = DATA;
  nbytes_data = (n >= 0)? n : pck.get_nbytes();
  mode = r;
  source = pck.get_source();
  target = pck.get_target();
  tid = pck.get_tid();
  msdu_id = pck.get_id();
  tx_power = p;
  net_all_vec = nav;
  ACKpol = apol;
  
  nbytes_overhead = service_field_overhead + data_packet_overhead;
  if(ACKpol == blockACK) nbytes_overhead += mpdu_delimiter_overhead;

  nbits = (nbytes_data + nbytes_overhead)*8;
  packet_duration = calc_duration (nbits, mode, addP);

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
    case BA:
          return os << "BA packet " << p.id << " from " << *(p.source) << " to "
                    << *(p.target);
    default:
    	return os << "unknown packet type";
  }
}

////////////////////////////////////////////////////////////////////////////////
// vector<long_integer> output operator <<                                    //
////////////////////////////////////////////////////////////////////////////////
ostream& operator << (ostream& os, const vector<long_integer>& vec) {
	for(typename vector<long_integer>::const_iterator k = vec.begin(); k != vec.end(); k++) {
      os << *k;
      if(k != vec.end() - 2) os << " ";
      else os << " and ";
	}
	return os;
}
