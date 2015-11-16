#ifndef _Terminal_private_h
#define _Terminal_private_h 1

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <utility>

#include "Position.h"
#include "Scheduler.h"
#include "Channel.h"
#include "Packet.h"
#include "timestamp.h"
#include "random.h"
#include "PHY.h"
#include "MAC.h"
#include "link_adapt.h"
#include "log.h"
#include "Traffic.h"

////////////////////////////////////////////////////////////////////////////////
// class Terminal_private                                                     //
//                                                                            //
// private members of class Terminal                                          //
////////////////////////////////////////////////////////////////////////////////
class Terminal_private{
protected:
  Scheduler* ptr2sch; // pointer to simulation scheduler
  random*    randgen; // pointer to random number generator
  log_file*  mylog;   // pointer to log file
  timestamp  transient_time; // collect results only after transient_time
  
  // pointers to lower layers
  PHY* myphy; 
  MAC* mymac;

  Position where; // terminal location
  unsigned id;    // unique identification number
  static unsigned nterm; // total number of instanciated terminals

  /////////////////////////////
  // performance measurements
  long_integer  n_tx_bytes;      // number of successfully transmitted bytes
  unsigned long n_tx_packets;    // number of successfully transmitted packets
  unsigned long n_pck_lost_q;  // number of packets lost due to queue overflow
  unsigned long n_pck_lost_r;  // number of packets lost after too many retries
  
  unsigned long n_att_packets;
  long_integer queue_length;
  
  timestamp     transfer_delay;        // sum of transfer delays
  double        transfer_delay_E2;     // sum of squared transfer delays
  timestamp     transmission_delay;    // sum of transmission delays
  double        transmission_delay_E2; // sum of squared transmission delays
  /////////////////////////////

  ////////////////////////////
  // private member functions
  virtual void connect(Terminal* t, adapt_struct ad, traffic_struct ts) = 0;
  // creates connection to terminal '*t'
  // using link adaptation parameters 'ad' and traffic parameters 'ts'
};


#endif
