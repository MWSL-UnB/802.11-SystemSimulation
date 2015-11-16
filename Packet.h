#ifndef _packet_h
#define _packet_h 1

#include "timestamp.h"
#include "long_integer.h"

class Terminal;

////////////////////////////////////////////////////////////////////////////////
// enum transmission_mode                                                     //
//                                                                            //
// rate adaptation method or transmission data rate                           //
////////////////////////////////////////////////////////////////////////////////
typedef enum {OPT,    // optimal genie-aided adaptative-rate scheme
              SUBOPT, // suboptimal transmitter-based scheme
              M0,     // dummy
              M6,     // fixed rate, 6Mbps
              M9,     //           , 9Mbps
              M12,    //           , 12Mbps
              M18,    //           , 18Mbps
              M24,    //           , 24Mbps
              M36,    //           , 36Mbps
              M48,    //           , 48Mbps
              M54     //           , 54Mbps
              } transmission_mode;

double tx_mode_to_double (transmission_mode tm);
// conversion to double

inline transmission_mode& operator--(transmission_mode& tm) {
    return tm = (tm <= M6)? tm : transmission_mode(tm-1);
}

inline transmission_mode& operator++(transmission_mode& tm) {
    return tm = (tm <= SUBOPT || tm == M54)? tm : transmission_mode(tm+1);
}

ostream& operator<< (ostream& os, const transmission_mode& tm);
istream& operator>> (istream& is, transmission_mode& tm);

////////////////////////////////////////////////////////////////////////////////
// enum packet_type                                                           //
////////////////////////////////////////////////////////////////////////////////
typedef enum {DUMMY, DATA, ACK, RTS, CTS} packet_type;


////////////////////////////////////////////////////////////////////////////////
// class Packet                                                               //
////////////////////////////////////////////////////////////////////////////////
class Packet {
protected:
  Terminal* source; // source terminal
  Terminal* target; // target terminal

  static long_integer packet_count;
  long_integer id;

  Packet() {id = packet_count++;}

public:
  long_integer      get_id ()        const {return id;}
  Terminal*         get_source()     const {return source;}
  Terminal*         get_target()     const {return target;}

};

////////////////////////////////////////////////////////////////////////////////
// class MSDU                                                                 //
////////////////////////////////////////////////////////////////////////////////
class MSDU : public Packet {
  timestamp time_created;
  timestamp tx_time;
  unsigned tid; // traffic identifier
  unsigned nbytes_data;     // number of data bytes  
  
public:
  MSDU(unsigned n = 0,            // number of data bytes
       Terminal* from = 0,        // source terminal
       Terminal* to = 0,          // target terminal
       unsigned priority = 0,     // traffic identifier
       timestamp gen_time = 0     // time that packet was generated
       );

  unsigned  get_nbytes()        const {return nbytes_data;}
  timestamp get_time_created()  const {return time_created;}
  timestamp get_tx_time()       const {return tx_time;}
  unsigned  get_tid()           const {return tid;}
  
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

public:
  MPDU(packet_type tp = DUMMY,    // packet type
       Terminal* from = 0,        // source terminal
       Terminal* to = 0,          // target terminal
       double p = 0,              // transmit power in dBm
       transmission_mode r = M0,  // transmission mode
       timestamp nav = 0          // NAV
       );

  transmission_mode get_mode()       const {return mode;}
  timestamp         get_duration ()  const {return packet_duration;}
  timestamp         get_nav()        const {return net_all_vec;}
  unsigned          get_nbits()      const {return nbits;}
  double            get_power ()     const {return tx_power;}
  packet_type       get_type()       const {return t;}       
  
  friend ostream& operator << (ostream& os, const MPDU& p);
  
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
            transmission_mode r = M6,
            timestamp nav = 0,
            unsigned priority = 0,
            unsigned frag = 1,
            unsigned nfrags = 1,
            unsigned mid = 0
           );

  DataMPDU (MSDU pck,
            int n = -1,
            unsigned frag = 1,
            unsigned nfrags = 1,                     
            double p = 0,
            transmission_mode r = M6,
            timestamp nav = 0
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
#endif
