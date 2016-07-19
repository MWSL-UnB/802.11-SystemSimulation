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

#ifndef _packet_h
#define _packet_h 1

#include "timestamp.h"
#include "long_integer.h"
#include <vector>

class Terminal;
class Standard;

////////////////////////////////////////////////////////////////////////////////
// enum transmission_mode                                                     //
//                                                                            //
// rate adaptation method or transmission data rate                           //
////////////////////////////////////////////////////////////////////////////////
typedef enum {OPT,    // optimal genie-aided adaptative-rate scheme
	SUBOPT, // suboptimal transmitter-based scheme
	MCS,     // dummy
	MCS0,	// MCSs
	MCS1,
	MCS2,
	MCS3,
	MCS4,
	MCS5,
	MCS6,
	MCS7,
	MCS8,
	MCS9
} transmission_mode;

typedef enum {
	noACK,
	normalACK,
	blockACK
} ACKpolicy;

inline transmission_mode& operator--(transmission_mode& tm) {
    return tm = (tm <= MCS0)? tm : transmission_mode(tm-1);
}

inline transmission_mode& operator++(transmission_mode& tm) {
    return tm = (tm <= SUBOPT || MCS9)? tm : transmission_mode(tm+1);
}

ostream& operator<< (ostream& os, const transmission_mode& tm);
istream& operator>> (istream& is, transmission_mode& tm);

////////////////////////////////////////////////////////////////////////////////
// enum packet_type                                                           //
////////////////////////////////////////////////////////////////////////////////
typedef enum {DUMMY, DATA, ACK, RTS, CTS, BA} packet_type;


////////////////////////////////////////////////////////////////////////////////
// class Packet                                                               //
////////////////////////////////////////////////////////////////////////////////
class Packet {
protected:
  Terminal* source; // source terminal
  Terminal* target; // target terminal

  static long_integer packet_count;
  long_integer id;

  Packet() {
	  id = packet_count++;
	  source = 0;
	  target = 0;
  }

public:
  long_integer      get_id ()        const {return id;}
  Terminal*         get_source()     const {return source;}
  Terminal*         get_target()     const {return target;}

};

////////////////////////////////////////////////////////////////////////////////
// class MSDU                                                                 //
////////////////////////////////////////////////////////////////////////////////
class MSDU : public Packet {
	unsigned nbytes_data;     // number of data bytes
	unsigned tid; // traffic identifier
	timestamp time_created;
	unsigned retry_count;		// number of MSDU retries
	timestamp tx_time;

public:
	MSDU(unsigned n = 0,            // number of data bytes
			Terminal* from = 0,        // source terminal
			Terminal* to = 0,          // target terminal
			unsigned priority = 0,     // traffic identifier
			timestamp gen_time = timestamp(0)     // time that packet was generated
	);

	timestamp get_time_created()  const {return time_created;}
	timestamp get_tx_time()       const {return tx_time;}
	unsigned  get_tid()           const {return tid;}
	unsigned  get_nbytes()        const {return nbytes_data;}
	unsigned	get_retry_count()	const {return retry_count;}
	unsigned	inc_retry_count()	{ return retry_count++;}

	void set_tx_time(timestamp t) {tx_time = t;}
};

////////////////////////////////////////////////////////////////////////////////
// class MPDU                                                                 //
////////////////////////////////////////////////////////////////////////////////
class MPDU : public Packet {
protected:
  unsigned nbytes_overhead; // number of MAC overhead bytes
  unsigned nbits;           // number of bits in a packet, including overhead

  transmission_mode mode;
  packet_type t;

  double tx_power; // transmit power in dBm

  timestamp packet_duration;
  timestamp net_all_vec; // NAV field

  // Used only for BA packets
  vector<long_integer> pcks2ACK;
  ACKpolicy ACKpol;

public:
  MPDU(packet_type tp = DUMMY,    // packet type
       Terminal* from = 0,        // source terminal
       Terminal* to = 0,          // target terminal
       double p = 0,              // transmit power in dBm
       transmission_mode r = MCS,  // transmission mode
       timestamp nav = timestamp(0)          // NAV
       );

  transmission_mode get_mode()       const {return mode;}
  timestamp         get_duration ()  const {return packet_duration;}
  timestamp         get_nav()        const {return net_all_vec;}
  unsigned          get_nbits()      const {return nbits;}
  double            get_power ()     const {return tx_power;}
  packet_type       get_type()       const {return t;}
  ACKpolicy			get_ACKpol()	 const {return ACKpol;}
  vector<long_integer> getPcks2Ack() const {return pcks2ACK;}

  friend ostream& operator << (ostream& os, const MPDU& p);
  friend ostream& operator << (ostream& os, const vector<long_integer>& vec);

  void setPcks2Ack(const vector<long_integer>& pcks2Ack);

  void setACKpol(ACKpolicy acKpol) {
	  ACKpol = acKpol;
  }
};

////////////////////////////////////////////////////////////////////////////////
// class DataMPDU                                                             //
////////////////////////////////////////////////////////////////////////////////
class DataMPDU : public MPDU {
  unsigned tid;
  unsigned frag_number;
  unsigned frag_total;
  unsigned msdu_id;
  unsigned nbytes_data;     // number of data bytes

public:
  DataMPDU (unsigned n,            // number of data bytes  
            Terminal* from = 0,
            Terminal* to = 0,
            double p = 0,
            transmission_mode r = MCS0,
            timestamp nav = timestamp(0),
            unsigned priority = 0,
            unsigned frag = 1,
            unsigned nfrags = 1,
            unsigned mid = 0,
			ACKpolicy apol = normalACK,
			bool addP = true
           );

  DataMPDU (MSDU pck,
            int n = -1,
            unsigned frag = 1,
            unsigned nfrags = 1,                     
            double p = 0,
            transmission_mode r = MCS0,
            timestamp nav = timestamp(0),
			ACKpolicy apol = normalACK,
			bool addP = true
           );
  // creates a DataMPDU inheriting parameters of a given MSDU 'pck'
                            
  unsigned get_nbytes()        const {return nbytes_data;}
  unsigned get_nbytes_mac()    const {return nbytes_data+nbytes_overhead;}
  unsigned get_tid()           const {return tid;}
  unsigned get_frag_number()   const {return frag_number;}
  unsigned get_frag_total()    const {return frag_total;}
  unsigned get_msdu_id()       const {return msdu_id;}
};

////////////////////////////////////////////////////////////////////////////////
inline timestamp ack_duration(transmission_mode tm) {
  return (MPDU(ACK, 0, 0, 0, tm)).get_duration();
}
inline timestamp ba_duration(transmission_mode tm) {
  return (MPDU(BA, 0, 0, 0, tm)).get_duration();
}
#endif
