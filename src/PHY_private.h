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

#ifndef _PHY_private_h
#define _PHY_private_h 1

#include "random.h"
#include "Packet.h"
#include "Scheduler.h"
#include "Position.h"
#include "log.h"
#include <valarray>
#include <cmath>

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

  double calculate_per(transmission_mode mode, double SNR) const;
  // returns packet error rate for a given transmission rate and signal-to-noise
  // ratio 'SNR' dB. The packet error rate is calculated based on a polynomial
  // approximation of the function log10(PER) x SNR.

  double calculate_SNReff(valarray<double> SNRps, double beta) const;
  // returns Effective SNR SNReff of subcarriers SNRs SNRps, calculated using
  // the exponential method with given beta parameter

};

#endif

