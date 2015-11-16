#ifndef _MAC_h
#define _MAC_h 1

#include "MAC_private.h"

////////////////////////////////////////////////////////////////////////////////
// struct mac_struct                                                          //
//                                                                            //
// MAC layer parameters                                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
struct mac_struct {
  unsigned retry;        // retry counter limit
  unsigned RTS_thresh;   // RTS treshold (packets with more than RTS_thresh
                         // bytes use RTS/CTS protocol)
  unsigned frag_thresh;  // fragmentation threshold, fragments must have less
                         // than frag_thresh bytes
  unsigned queue_size;   // size of packet queue                                                

  mac_struct(unsigned r, unsigned rt, unsigned ft, unsigned qs)
            : retry(r), RTS_thresh(rt), frag_thresh(ft), queue_size(qs)  {}
};

////////////////////////////////////////////////////////////////////////////////
// class MAC                                                                  //
//                                                                            // 
// implements the MAC layer of IEEE 802.11a (DCF only).                       //
//                                                                            //
// Usage:                                                                     //
//                                                                            //
//  A MAC object must belong to a Terminal object. A PHY object must be       //
//  assigned to the MAC through function "connect"                            //
//                                                                            //
//  The MAC object can schedule a packet for transmission using               //
//  'macUnitdataReq'.                                                         //
//                                                                            //
//  The MAC object receives a packet through the function 'receive'.          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
class MAC : MAC_private {

public:
  MAC(Terminal* t,    // pointer to owner terminal
      Scheduler* s,   // pointer to simulation scheduler
      random *r,      // pointer to random number generator
      log_file *l,    // pointer to log
      mac_struct mac  // MAC layer parameters
     );

  void phyCCA_busy();
  void phyCCA_free();
  // MAC receives message (from PHY) that channel has become busy/free

  void connect (PHY* p) {myphy = p;}
  // associates PHY object '*p' to this object.
  
  unsigned long get_n_packets_att() const {return n_att_frags;}
  // returns number of attenpted data fragment transmissions
  
  double get_tx_data_rate() const  {return (tx_data_rate/n_att_frags);}
  // returns average transmission data rate in Mbps
 
  void phyRxEndInd(MPDU p);
  // receive packet 'p' (from PHY)

  unsigned macUnitdataReq(MSDU p);
  // attempt to transmit MSDU 'p', put it in queue
  // returns queue size
  
};

#endif
