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

#ifndef _channel_h
#define _channel_h 1

#include "Channel_private.h"

////////////////////////////////////////////////////////////////////////////////
// struct channel_struct                                                      //
//                                                                            //
// channel parameters                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
struct channel_struct {

  double loss_exponent;  // path loss exponent in dB
  double ref_loss;       // reference path loss at 1m distance in dB
  double doppler_spread; // maximum Doppler spread in Hz
  unsigned number_sines; // number of sinewaves for Jakes' model
  channel_model model;	 // channel propagation model

  channel_struct(double le, double rl, double ds, unsigned ns, channel_model cmod)
            : loss_exponent(le), ref_loss(rl), doppler_spread(ds),
              number_sines(ns), model(cmod) {}
};

////////////////////////////////////////////////////////////////////////////////
// class Channel                                                              //
//                                                                            //
//  This class models a wireless channel with packet transmission.            //
//  It is modelled as a time-variant frequency-nonselective Rayleigh-fading   //
//  channel, using Jakes' model.                                              //
//  Interference among simultaneously transmitted packets is also considered. //
//  The long-term average path loss (without Rayleigh fading) is given by:    //
//    PL_dB = RefLoss_dB + 10*LossExponent*log10(d),                          //
//  with d the distance in meters.                                            //
//                                                                            //
//  Usage:                                                                    //
//                                                                            //
//  - Before using this class, the channel must be informed about existing    //
//    communication links between different terminals with the function       //
//    'new_link'.                                                             //
//                                                                            //
//  - A packet may be sent over the channel using the functions               //
//    'send_packet_one' and 'send_packet_all'. If the former is used, then    //
//    the packet will be received only by the PHY of the terminal in the      //
//    target field of the packet. If 'send_packet_all' is employed, then the  //
//    packet will be received by all PHYs known to the channel.               //
//    In both cases the packet will cause interference on all other packets   //
//    being  transmitted.                                                     //
//                                                                            //
//  - The channel may be asked about the current interference level at a      //
//    given PHY with the function 'get_interf_dBm'. The current path loss     //
//    between two PHYs is given by 'get_path_loss'.                           //
//                                                                            //
//  - To avoid time-consuming repeating calls to 'get_interf_dBm', a terminal //
//    can ask the channel to inform it when the channel occupation has        //
//    changed, either when a new packet is transmitted over the channel       //
//    ('busy_channel_request') or a packet stops being transmitted            //
//    ('free_channel_request').                                               //
//    The channel keeps lists of all the PHYs requiring notification. By      //
//    calling 'busy_channel_remove' or 'free_channel_remove' the PHY can be   //
//    removed from these lists.                                               //
//                                                                            //
//  Remarks:                                                                  //
//  . non-active links are static, active links have Rayleigh fading          //
//  . interference is modelled as highest interference during a packet        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
class Channel : Channel_private {

public:
  Channel(Scheduler *s,  // pointer to simulation scheduler
          random *r,
          channel_struct p,
          log_file* l
          );
  ~Channel() {};

  void new_link(PHY* pt1, PHY* pt2);
  // creates an active time-variant channel link between two terminals
  // it is ignored if link already exists

  void busy_channel_request(PHY* p) {waiting_list_busy.push_back(p);}  
  // PHY '*p' asks to be notified when any new packet occupies the channel

  void free_channel_request(PHY* p) {waiting_list_free.push_back(p);}
  // PHY '*p' asks to be notified whe any packet stops being transmitted
  
  void busy_channel_remove(PHY* p) {waiting_list_busy.remove(p);}
  void free_channel_remove(PHY* p) {waiting_list_free.remove(p);}
  // PHY '*p' cancels notification request

  void send_packet_one(MPDU pack);
  // send packet 'pack', it will be received just by target terminal
  void send_packet_all(MPDU pack);
  // send packet 'pack', it will be received by all terminals

  double get_interf_dBm(PHY* t);
  // returns interference level in dBm at a given PHY
  
  double get_path_loss(PHY *t1, PHY *t2);
  // returns path loss in dB between two given PHYs

};
#endif
