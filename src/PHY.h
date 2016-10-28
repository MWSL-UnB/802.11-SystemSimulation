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

#ifndef _PHY_h
#define _PHY_h 1

#include "PHY_private.h"


////////////////////////////////////////////////////////////////////////////////
// struct PHY_struct                                                          //
//                                                                            //
// physical layer parameters                                                  //                                                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
struct PHY_struct {
  double NoiseVar;  // noise variance at receiver in dBms
  double Sens;      // carrier sensitivity level in dBms

  PHY_struct(double N, double S): NoiseVar(N), Sens(S) {}
};


////////////////////////////////////////////////////////////////////////////////
// class PHY                                                                  //
//                                                                            //
// implements the physical layer of IEEE 802.11a.                             //
//                                                                            //
// Usage:                                                                     //
//                                                                            //
//  A PHY object must belong to a Terminal object. A MAC object must be       //
//  assigned to the PHY through function "connect"                            //
//                                                                            //
//  A packet can be sent through the PHY object using the function            //
//  "phyTxStartReq". The packet is then forwarded by PHY to the wireless      //
//  channel.                                                                  //
//                                                                            //
//  The wireless channel delivers packets to PHY using function "receive".    //
//  Each one of these packets has a path loss and an interference level       //
//  determined by the channel. PHY decides then if the packet is received     //
//  correctly, and if this is the case forwards it to the MAC layer.          //
//  The error model is based on bit error rate results obtained through       //
//  simulations for an AWGN channel. The packet error rate is obtained by     //
//    PER = 1 - pow((1-BER/L),N),                                             //
//  where N is the number of bits in a packet and L is the expected error     //
//  burst length, which is employed to approximate the coding effects.        //
//                                                                            //
//  Functions 'carrier_sensing', 'notify_busy_channel', 'notify_free_channel' //
//  'cancel_notify_busy_channel', 'cancel_notify_free_channel' can be used    //
//  by MAC layer to determine whether and when the channel is busy or free.   //
//  The channel is considered to be free at a particular PHY if the total     //
//  interference level is less than the parameter carrier sensitivity level.  //
//                                                                            //
//  Functions 'channel_occupied' and 'channel_released' should be called by   //
//  channel to inform PHY if a new packet is occupying the channel or if a    //
//  packet stopped transmitting.                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
class PHY : private PHY_private {
public:
  PHY(Terminal* t,  // pointer to owner terminal
      Position p,   // terminal position
      Channel* c,   // pointer to wireless channel used
      random* r,    // pointer to random number generator
      Scheduler* s, // pointer to simulation scheduler
      log_file* l,  // pointer to log
      PHY_struct ps // struct with physical layer parameters
      );

  bool carrier_sensing();
  // returns true if channel is sensed to be busy by PHY, i.e., if interference
  // level is higher than the carrier sensitivity level;
  // returns false otherwise.

  void channel_occupied(double interf);
  // PHY receives message (from channel) that a new packet is being transmitted.
  // Inform MAC if new interference is greater than sensitivity level.

  void channel_released(double interf);
  // PHY receives message (from channel) that a packet stopped being transmitted.
  // Inform MAC if new interference is less than sensitivity level.

  void connect(MAC* m) {mymac = m;}
  // associate MAC object 'm' to this object.

  unsigned get_id() const {return id;}
  Position get_pos() const {return pos;}

  void notify_busy_channel();
  void notify_free_channel();
  // MAC requests notification when channel becomes free.

  void cancel_notify_busy_channel();
  void cancel_notify_free_channel();
  // MAC cancels notification request.

  void receive(MPDU p, valarray<double> path_loss, double interf = 0);
  // a packet 'p' is received with path loss 'path_loss' dB and interference level
  // 'interf' mW. If packet is received correctly, forward it to MAC layer.

  void phyTxStartReq(MPDU p, bool to_all);
  // send packet 'p' to wireless channel. If 'to_all' is true, then packet will
  // be received by all terminals in network. If 'to_all' is false, then only
  // target terminal will receive the packet.

  transmission_mode opt_mode(Terminal* t1, unsigned pack_len,
                             double per_target, double power);
  // returns optimal transmission mode, i.e., such that the a packet with
  // 'pack_len' data bytes and transmit power 'power' dBm is received by
  // terminal '*t1' with a packet error rate less than 'per_target'

  double opt_power(Terminal* t1, unsigned pack_len, double per_target,
                   transmission_mode mode, double pmin, double pmax,
                   double pstep);
  // returns optimal transmission power, i.e., such that the a packet with
  // 'pack_len' data bytes and transmission mode 'mode' is received by
  // terminal '*t1' with a packet error rate less than 'per_target'
  
  double get_energy () const {return energy;}
  // returns the transmission energy consumed so far in mW*s
  
  friend ostream& operator << (ostream& os, const PHY& p);
};

valarray<double> to_dB(valarray<double> linArray);
double to_dB(double linArray);

valarray<double> from_dB(valarray<double> linArray);
double from_dB(double linArray);

#endif
