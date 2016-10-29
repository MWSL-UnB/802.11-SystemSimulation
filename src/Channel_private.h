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

#ifndef _channel_private_h
#define _channel_private_h 1

#include <complex>
#include <utility>
#include <map>
#include <list>
#include <valarray>

#include "Packet.h"
#include "Scheduler.h"
#include "timestamp.h"
#include "long_integer.h"
#include "random.h"
#include "log.h"

class PHY;

typedef enum{
	A,
	B,
	C,
	D,
	E,
	F
} channel_model;

ostream& operator<< (ostream& os, const channel_model& cm);
istream& operator>> (istream& is, channel_model& cm);

////////////////////////////////////////////////////////////////////////////////
// class term_pair                                                            //
//                                                                            //
// a pair of pointers to PHY, with comparison operators to use it with 'map'  //
////////////////////////////////////////////////////////////////////////////////
typedef pair<PHY*, PHY*> tp_type;

class term_pair {
private:
  tp_type tp;
  
public:
  term_pair() {};
  term_pair(PHY* t1, PHY* t2);

  bool operator< (const term_pair& p) const;
  bool operator== (const term_pair& p) const;

  friend ostream& operator << (ostream& os, const term_pair& t);
};

////////////////////////////////////////////////////////////////////////////////
// class Link                                                                 //
//                                                                            //
// implements a time-variant channel link between two PHYs.                   //
// Rayleigh fading is implemented using Jakes' simulation model, in which the //
// channel gain is the sum of a number of sinewaves with different phases and //
// frequencies:                                                               //
//   g(t) = x(t) + jy(t),                                                     //
// where                                                                      //
//   x(t) = 1/sqrt(M+1/2) [\sum_{1}^{M}cos(beta_n)cos(omega_n*t+theta_n)      //
//                         + sqrt(2)cos(alpha)*cos(2*pi*f_d*t)]               //
// and                                                                        //
//   x(t) = 1/sqrt(M+1/2) [\sum_{1}^{M}sin(beta_n)cos(omega_n*t+theta_n)      //
//                         + sqrt(2)sin(alpha)*cos(2*pi*f_d*t)],              //
// with f_d the maximum Doppler spread, M the number of sinewaves,            //
//   beta_n = pi*n/M                                                          //
// and                                                                        //
//   omega_n = 2*pi*f_d*cos(2*pi*n)/M                                         //
// theta_n and alpha are selected randomly and are different for each link,   //
// so that independent fading processes can be generated for each link.       //
//                                                                            //
// Usage:
//  - when constructor is called with parameters, channel link is initialized //
//    with random values for theta_n and alpha. Each time the function 'fade' //
//    is called the channel gain will be updated and the new value returned.  //
////////////////////////////////////////////////////////////////////////////////
class Link {
  term_pair terms; // linked terminals

  ////////////////////////////
  // Jakes' model parameters
  valarray<double> cosbeta;
  valarray<double> sinbeta;
  valarray<double> omega;
  valarray<double> theta;
  double cosalpha;
  double sinalpha;

  unsigned n_osc; // number of sine waves

  double doppler_spread; // maximum Doppler spread in Hz

  valarray<double> carrier_loss; // path loss for each subcarrier
  double path_loss;      // current average subcarrier path loss in dB
  double path_loss_mean; // average path loss (without fading) in dB 

  double time_diff_min; // link gain is not updated if last update 
                        // happened less than 'time_diff_min' seconds ago
                        
  timestamp time_last; // time of latest link gain update

public:
  //Link() {}
  Link(term_pair t,      // pair of linked terminals
       double path_loss, // mean path loss in dB
       double fd,        // maximum Doppler spread in Hz
       random* r,        // pointer to random number generator
       unsigned ns       // number of sinewaves in Jakes' model
       );

  double fade(timestamp t); // returns the link gain amplitude at time 't' in dB

  bool belong(term_pair t) const {return t == terms;}
  // returns true if this link corresponds to 't', false otherwise

  valarray<double> get_carrier_loss() const {return carrier_loss;};

  friend ostream& operator << (ostream& os, const Link& l);
  // output operator
};

////////////////////////////////////////////////////////////////////////////////
// struct pack_struct                                                         //
//                                                                            //
// structure containing a packet and the interference level to this packet at //
// the target terminal                                                        //
////////////////////////////////////////////////////////////////////////////////
struct pack_struct {
  MPDU pck;
  double interf;
  double interf_max;

  pack_struct(MPDU p): pck(p), interf(0), interf_max(0) {}
  
  pack_struct(MPDU p, double d): pck(p), interf(d), interf_max(d) {}

  pack_struct(MPDU p, double d1, double d2)
             : pck(p), interf(d1), interf_max(d2) {}
};

////////////////////////////////////////////////////////////////////////////////
// class Channel_private                                                      //
//                                                                            //
//  implements private members of class Channel                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
class Channel_private {
protected:
  Scheduler *ptr2sch; // pointer to scheduler
  random *rand_gen;  // pointer to random number generator

  log_file*  mylog;
  bool       logflag;  // true if Channel events should be logged

  //////////////////////
  // channel parameters
  double LossExponent;
  double RefLoss_dB;
  double DopplerSpread_Hz;
  unsigned NumberSinus;
  channel_model cModel;

  vector<PHY*> term_list; // list of all active terminals
  vector<Link> links;     // list of all active links

  list<pack_struct> air_pack; // list of packets currently transmitted

  map<term_pair,double> path_loss;
  // channel gains between all terminals, including both active links and those
  // that cause only interference

  list<PHY*> waiting_list_free; // list of terminals that requested notification
                                // when channel is released
  list<PHY*> waiting_list_busy; // list of terminals that requested notification
                                // when channel is occupied


  void busy_channel_message(MPDU pck);
  void free_channel_message(MPDU pck);
  // tell all the terminals requesting notification that channel was
  // occupied/released by packet 'pck'

  void new_term(PHY* t); // adds new terminal to the channel
  
  void stop_send_all(long_integer pack_id);
  void stop_send_one(long_integer pack_id);
  // Packet with identification 'pack_id' stopped being transmitted

public:
  virtual ~Channel_private() {};

  virtual double get_interf_dBm(PHY* t) = 0;
  // returns interference level in dBm at PHY '*t'

  ////////////////////////////////////
  // wrapper functions for call-back
  static void wrapper_to_stop_send_all (void* p, long_integer i) {
                                      ((Channel_private*) p)->stop_send_all(i);}
  static void wrapper_to_stop_send_one (void* p, long_integer i) {
                                      ((Channel_private*) p)->stop_send_one(i);}
};

#endif
