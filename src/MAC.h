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
// implements the MAC layer of IEEE 802.11a (DCF only) with 802.11e EDCA      //
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
      mac_struct mac, // MAC layer parameters
	  accCat AC	  // Access category of station
     );

  void phyCCA_busy();
  void phyCCA_free();
  // MAC receives message (from PHY) that channel has become busy/free

  void connect (PHY* p) {myphy = p;}
  // associates PHY object '*p' to this object.
  
  unsigned long get_n_packets_att() const {return n_att_frags;}
  // returns number of attenpted data fragment transmissions
  
  double get_tx_data_rate() const  {if(n_att_frags > 0) return (tx_data_rate/n_att_frags);
  	  	  	  	  	  	  	  	  	else return 0.0;}
  // returns average transmission data rate in Mbps
 
  void phyRxEndInd(MPDU p);
  // receive packet 'p' (from PHY)

  unsigned macUnitdataReq(MSDU p);
  // attempt to transmit MSDU 'p', put it in queue
  // returns queue size
  
  friend ostream& operator << (ostream& os, const accCat AC);
  // Overload of output operator for accCat types

};

#endif
