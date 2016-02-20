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

#ifndef _Terminal_h
#define _Terminal_h 1

#include "Terminal_private.h"


////////////////////////////////////////////////////////////////////////////////
// class Terminal                                                             //
//                                                                            //
// abstract base class for all kinds of terminals                             //
//                                                                            //
// The Terminal class is responsible for the traffic generation and for       //
// storing the performance statistics.
//
// Usage:
// - after instantiation, connections between terminals are created by calls  //
//   to the function 'connect_two'. After all connections have been           //
//   established, a call to 'start_traffic' begins the packet generation.     //
// - generated packets are forwarded to the MAC class. The Terminal receives  //
//   status indications from the MAC (see functions 'macUnitdata...'),        //
//   stating whether packets are correctly transmitted or not.                //
// - the MAC object can inform the Terminal about transmission of individual  //
//   fragments, in order to allow link adaptation. This is done throug the    //
//   functions 'la_fail', 'la_success' and 'la_rx_success'. The adaptive link //
//   parameters can be obtained by calling 'get_power' and 'get_current_mode' //
////////////////////////////////////////////////////////////////////////////////
class Terminal : public Terminal_private{
public:
  Terminal(Position p,            // terminal location
           Scheduler* s,          // pointer to simulation scheduler
           Channel* c,            // pointer to wireless channel
           random* r,             // pointer to random number generator
           log_file* l,           // pointer to log file
           mac_struct mac,        // MAC layer parameters
           PHY_struct phy,        // physical layer parameters
           timestamp tr
          );
  ~Terminal();

  friend void connect_two (Terminal* t1, accCat AC1, Terminal* t2, accCat AC2,
		  	  	  	  	   Channel* ch, adapt_struct ad,
                           traffic_struct tr1to2, traffic_struct tr2to1);
  // establishes connection between two terminals through wireless channel '*ch'
  // performs link adaptation using parameters 'ad'

  virtual string get_connections() const = 0;
  // returns string with terminals connected to this one

  unsigned get_id() const {return id;}
  // returns unique terminal identification number

  Position get_pos() const {return where;}
  // returns terminal location

  MAC* get_mac () const {return mymac;}
  // returns a pointer to the MAC layer
    
  PHY* get_phy () const {return myphy;}
  // returns a pointer to the physical layer

  /////////////////////////////////////////////////
  // return simulation results for this terminal    
  double get_average_power() const;  // average transmission power in dBm
  double get_packet_loss_rate() const; // packet loss rate due to retry limit
  double get_overflow_rate() const; // packet loss rate due to queue overflow
  double get_queue_length() const; // average MAC queue length
  double get_transfer_delay() const; // average transfer delay in sec
  double get_transfer_delay_std() const; // standard deviation of transfer delay
  double get_transmission_delay() const; // average transmission delay in sec
  double get_transmission_delay_std() const; // standard deviation of tx delay
  double get_tx_data_rate() const; // average transmission rate of PHY in Mbps
  long_integer  get_n_bytes() const; // number of bytes successfully sent
  unsigned long get_n_packets() const; // number of MSDUs successfully sent
  unsigned long get_n_packets_att() const; // number of attempted MPDUs
  /////////////////////////////////////////////////

  //////////////////////////////
  // link adaptation functions 
  virtual void la_failed(Terminal* t) = 0;
  // ACK timed out, transmission failed, adapt link
  
  virtual void la_rts_failed(Terminal* t) = 0;
  // CTS timed out, transmission failed, adapt link
  
  virtual void la_success(Terminal* t, bool lastfrag) = 0;
  // ACK received, transmission succeeded, adapt link.
  // Adaptive link parameters may change only with last fragment, i.e., if 
  // "lastfrag" is true.
  
  virtual void la_rx_success(Terminal* t, transmission_mode rx_mode) = 0;
  // data packet received successfully with data rate 'rx_mode', adapt link

  virtual transmission_mode get_current_mode(Terminal* t,unsigned pl) = 0;
  // returns (optimized) data rate for transmission from this terminal to 't'
  // considering a packet with 'pl' data bytes

  virtual double get_power(Terminal* t, unsigned pl) = 0;
  // returns (optimized) power in dBm for transmission from this terminal to 't'

  //////////////////////////////


  //////////////////////////////
  // MAC status indications 
  void macUnitdataStatusInd(MSDU p, timestamp ack_delay);
  // ACK received, packet 'p' was transmitted correctly.
  // 'ack_delay' is the time interval between packet reception at target and
  // ACK reception at source.
  void macUnitdataInd(MSDU p) {}
  // packet 'p' was received 
  void macUnitdataQueueOverflow(MSDU p) {++n_pck_lost_q;}
  // packet 'p' was not transmitted due to queue overflow in MAC
  void macUnitdataMaxRetry(MSDU p) {++n_pck_lost_r;}
  // packet 'p' could not be transmitted after maximum number of retransmissions

  //////////////////////////////

  
  virtual void macUnitdataReq(MSDU p) = 0;
  
  virtual string str() const = 0;
  // returns string with terminal type and identification

  timestamp time() const {return ptr2sch->now();}                           
  // returns current simulation time
                           
};

ostream& operator<< (ostream& os, const Terminal& t);


////////////////////////////////////////////////////////////////////////////////
// class MobileStation                                                        //
//                                                                            //
// a MobileStation is a terminal with  a single connection                    //
////////////////////////////////////////////////////////////////////////////////
class MobileStation : public Terminal {
  Terminal* connected; // active connection
  link_adapt la;   // link adaptation
  Traffic* tr;      // traffic generator
  accCat myAC;     // access category of this Mobile Station
  
  void connect(Terminal* t, adapt_struct ad, traffic_struct ts, accCat AC);
  // creates connection to terminal '*t'
  // using link adaptation parameters 'ad' and traffic parameters 'ts'
  
public:
  MobileStation(Position p, Scheduler* s, Channel* c, random* r, log_file* l,
                  mac_struct mac, accCat AC, PHY_struct phy, timestamp tr);
  ~MobileStation();
  
  /////////////////////////////////////////////////////////////
  // definition of virtual functions from base class Terminal
  string get_connections() const;
  
  double get_power(Terminal* t, unsigned pl) {return la.get_power(pl);}
  
  transmission_mode get_current_mode (Terminal* t, unsigned pl) {
                                                return la.get_current_mode(pl);}

  void la_failed(Terminal* t) {la.failed();}
  void la_rts_failed(Terminal* t) {la.rts_failed();}
  void la_rx_success(Terminal* t, transmission_mode rx_mode) {
                                                 return la.rx_success(rx_mode);}
  void la_success(Terminal* t, bool lastfrag) {la.success(lastfrag);}

  void macUnitdataReq(MSDU p);

  string str() const;
  /////////////////////////////////////////////////////////////
  
};

////////////////////////////////////////////////////////////////////////////////
// class AccessPoint                                                          //
//                                                                            //
// an AccessPoint is a Terminal with several possible connections             //
////////////////////////////////////////////////////////////////////////////////
class AccessPoint : public Terminal {
  map<Terminal*, tuple<link_adapt, Traffic*, accCat> >connection;
  // link adaptation units, traffic generators and AC for each connection

  void connect(Terminal* t, adapt_struct ad, traffic_struct ts, accCat AC);
  // creates connection to terminal '*t'
  // using link adaptation parameters 'ad' and traffic parameters 'ts'

public:
  AccessPoint(Position p, Scheduler* s, Channel* c, random* r, log_file* l,
                mac_struct mac, PHY_struct phy, timestamp tr);
  ~AccessPoint();             

  /////////////////////////////////////////////////////////////
  // definition of virtual functions from base class Terminal
  string get_connections() const;
  
  transmission_mode get_current_mode(Terminal* t, unsigned pl);
  double get_power(Terminal* t, unsigned pl);
  
  void la_failed(Terminal* t) {(get<0>(connection[t])).failed();}
  // transmission failed, adapt link
  
  void la_rts_failed(Terminal* t) {(get<0>(connection[t])).rts_failed();}
  // RTS transmission failed, adapt link
  
  void la_success(Terminal* t,  bool lastfrag) {
                                       (get<0>(connection[t])).success(lastfrag);}
  // transmission succeeded, adapt link
  
  void la_rx_success(Terminal* t, transmission_mode rx_mode) {
                                     (get<0>(connection[t])).rx_success(rx_mode);}
  // reception succeeded, adapt link

  void macUnitdataReq(MSDU p);

  string str() const;
  /////////////////////////////////////////////////////////////  
};
#endif
