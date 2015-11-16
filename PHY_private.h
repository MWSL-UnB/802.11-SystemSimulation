#ifndef _PHY_private_h
#define _PHY_private_h 1

#include "random.h"
#include "Packet.h"
#include "Scheduler.h"
#include "Position.h"
#include "log.h"

class Terminal;
class MAC;
class Channel;

////////////////////////////////////////////////////////////////////////////////
// class PHY_private                                                          //
//                                                                            //
// declares private member objects and functions of class PHY                 //
////////////////////////////////////////////////////////////////////////////////
class PHY_private {
protected:
  Terminal* term;     // pointer to owner terminal
  MAC* mymac;         // pointer to associated MAC layer
  Scheduler* ptr2sch; // pointer to simulation scheduler
  random* rand_gen;   // pointer to random number generator
  Channel* ch;        // pointer to wireless channel

  log_file*  mylog;
  bool       logflag;

  Position pos;       // transceiver location

  static unsigned nphys; // number of instanced PHYs
  unsigned id;           // unique identification number

  double NoiseVariance_dBm;
  double CCASensitivity_dBm; // carrier sensitivity level

  double energy;

  timestamp busy_begin, busy_end;
  // timestamps determining when transceiver is busy, to avoid simultaneous
  // transmission and/or reception of different packets by same transceiver.

  double calculate_ber(transmission_mode mode, double SNR) const;
  // returns bit error rate for a given transmission rate and sinal-to-noise
  // ratio 'SNR' dB. The bit error rate is calculated based on a polynomial
  // approximation of the function log10(BER) x SNR.
};

#endif

