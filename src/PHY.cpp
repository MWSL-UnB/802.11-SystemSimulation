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

#include "Terminal.h"
#include "Channel.h"
#include "PHY.h"
#include "Profiler.h"
#include "Standard.h"

#include <math.h>

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// error-model constants                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
const double burst_length = 3.3;
// expected error-burst length after convolutional decoding


const double min_thresh[9] = {-2.5103,  0.5000,  3.7609,
                               5.5103, 9.2712 , 12.5206,
							   14.5321, 11.6188, 15.4106};
const double max_thresh[9] = { 1.9897,  5.0000,  8.2609,
                              10.5103, 14.7712, 18.5206,
							  20.0321, 21.5000, 22.5000};

const int n_coeff = 5;
const double coeff[9][5] = {
             {  -2.2353000, -1.0721000,-0.1708900, 0.0243860, 0.0096656},
             {  -0.4517200, -0.3560888, 0.0627930,-0.0651410, 0.0064799},
             {  -0.3082200, -0.2063400, 0.1547100,-0.0389730, 0.0018157},
             {   2.6965000, -1.9353000, 0.4736600,-0.0509360, 0.0016224},
             {  34.8692000,-13.9070000, 2.0328000,-0.1283100, 0.0028499},
             {  93.9622000,-26.7075000, 2.8106000,-0.1290100, 0.0021372},
			 {-120.1972000, 26.3772000,-2.1564000, 0.0787190,-0.0011189},
			 { -42.9748720, 11.0385340,-1.0249930, 0.0411790,-0.0006080},
			 { -159.279240, 33.9061380,-2.6679000, 0.0922430,-0.0011850}};

const int n_coeff_high = 2;
const double coeff_high[9][2] = { {-2.3974,-1.1580}, { 2.1138,-1.3738}, { 7.7079,-1.5347},
                                  { 9.2576,-1.3244}, {11.3789,-1.1004}, {14.6479,-1.0454},
								  {20.0742,-1.2278}, {21.2886,-1.2977}, {18.1224,-0.9725}};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class PHY                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

unsigned PHY_private::nphys = 0;

////////////////////////////////////////////////////////////////////////////////
// PHY constructor                                                            //
////////////////////////////////////////////////////////////////////////////////
PHY::PHY(Terminal* t,
         Position p,
         Channel* c,
         random* r,
         Scheduler* s,
         log_file* l,
         PHY_struct ps) {

    term = t;
    pos = p;
    ch = c;
    rand_gen = r;
    ptr2sch = s;
    mylog = l;
    logflag = (*mylog)(log_type::phy);
    
    NoiseVariance_dBm = ps.NoiseVar;
    CCASensitivity_dBm = ps.Sens;

    busy_begin = busy_end = timestamp(0);
    id = nphys++;
    
    energy = 0;
}

////////////////////////////////////////////////////////////////////////////////
// PHY_private::calculate_ber                                                 //
//                                                                            //
// returns bit error rate for a given transmission rate and sinal-to-noise    //
// ratio 'SNR' dB. The bit error rate is calculated based on a polynomial     //
// approximation of the function log10(BER) x SNR.                            //
// Depending on the SNR, one of three different polynomials is used.          //
////////////////////////////////////////////////////////////////////////////////
double PHY_private::calculate_ber(transmission_mode mode, double SNR) const {
BEGIN_PROF("PHY::calculate_ber")

  double ber;

  unsigned index = mode - MCS0;

  if (SNR < min_thresh[index]) {
    // if SNR is low, then consider BER = 0.5
    ber = .5;

  } else if (SNR > max_thresh[index]) {
    // if SNR is high then use polynomial of order 'n_coeff_high - 1'
    double berlog = 0;

    double auxpow = 1.0;
    for (int i = 0; i < n_coeff_high; i++) {
      berlog += auxpow * coeff_high[index][i];
      auxpow = auxpow * SNR;
    }
    ber = pow(10.0,berlog);

  } else {
    // if SNR is medium then use polynomial of order 'n_coeff - 1'
    double berlog = 0;

    double auxpow = 1.0;
    for (int i = 0; i < n_coeff; i++) {
      berlog += auxpow * coeff[index][i];
      auxpow = auxpow * SNR;
    }
    ber = pow(10.0,berlog);
  }

END_PROF("PHY::calculate_ber")
return ber;
}

////////////////////////////////////////////////////////////////////////////////
// PHY::cancel_notify_busy_channel                                            //
//                                                                            //
// MAC cancels busy-channel notification request.                             //
////////////////////////////////////////////////////////////////////////////////
void PHY::cancel_notify_busy_channel() {
  ch->busy_channel_remove(this);
}

////////////////////////////////////////////////////////////////////////////////
// PHY::cancel_notify_free_channel                                            //
//                                                                            //
// MAC cancels free-channel notification request.                             //
////////////////////////////////////////////////////////////////////////////////
void PHY::cancel_notify_free_channel() {
  ch->free_channel_remove(this);
}

////////////////////////////////////////////////////////////////////////////////
// PHY::channel_occupied                                                      //
//                                                                            //
// PHY receives message that a new packet is being transmitted. Inform MAC if //
// new interference is greater than sensitivity level.                        //
////////////////////////////////////////////////////////////////////////////////
void PHY::channel_occupied(double interf) {
BEGIN_PROF("PHY::channel_occupied")

  if (interf >= CCASensitivity_dBm) {
    notify_free_channel();
    mymac->phyCCA_busy();
  }

END_PROF("PHY::channel_occupied")
}

////////////////////////////////////////////////////////////////////////////////
// PHY::channel_released                                                      //
//                                                                            //
// PHY receives message that a packet stopped being transmitted. Inform MAC   //
// if new interference is less than sensitivity level.                        //
////////////////////////////////////////////////////////////////////////////////
void PHY::channel_released(double interf) {
BEGIN_PROF("PHY::channel_released")

  if (interf < CCASensitivity_dBm) {
    ch->free_channel_remove(this);
    mymac->phyCCA_free();
  }
END_PROF("PHY::channel_released")
}

////////////////////////////////////////////////////////////////////////////////
// PHY::carrier_sensing                                                       //
//                                                                            //
// returns true if channel is sensed to be busy by PHY, i.e., if interference //
// level is higher than the carrier sensitivity level;                        //
// returns false otherwise.                                                   //
////////////////////////////////////////////////////////////////////////////////
bool PHY::carrier_sensing() {
  if (ch->get_interf_dBm(this) >= CCASensitivity_dBm) return true;
  else return false;
}

////////////////////////////////////////////////////////////////////////////////
// PHY::notify_busy_channel                                                   //
//                                                                            //
// MAC requests notification when channel becomes free.                       //
////////////////////////////////////////////////////////////////////////////////
void PHY::notify_busy_channel() {
BEGIN_PROF("PHY::notify_busy_channel")

  ch->free_channel_remove(this);
  ch->busy_channel_request(this);

END_PROF("PHY::notify_busy_channel")
}

////////////////////////////////////////////////////////////////////////////////
// PHY::notify_free_channel                                                   //
//                                                                            //
// MAC requests notification when channel becomes busy.                       //
////////////////////////////////////////////////////////////////////////////////
void PHY::notify_free_channel() {
BEGIN_PROF("PHY::notify_free_channel")

  ch->busy_channel_remove(this);
  ch->free_channel_request(this);

END_PROF("PHY::notify_free_channel")
}

////////////////////////////////////////////////////////////////////////////////
// PHY::opt_mode                                                              //
//                                                                            //
// returns optimal transmission mode, i.e., such that the a packet with       //
// 'pack_len' data bytes and transmit power 'power' dBm is received by        //
// terminal '*t1' with a packet error rate less than 'per_target'             //
////////////////////////////////////////////////////////////////////////////////
transmission_mode PHY::opt_mode(Terminal* t1, unsigned pack_len,
                                double per_target, double power) {
BEGIN_PROF("PHY::opt_mode")

  transmission_mode mode = Standard::get_maxMCS();
  unsigned nbits = (DataMPDU(pack_len)).get_nbits();
  double SNR = power - ch->get_path_loss(t1->get_phy(), this)
                     - NoiseVariance_dBm;

  for(;;) {
    if (mode == MCS0) break;

    double ber = calculate_ber(mode, SNR);
    double per = 1.0 - pow(1.0 - ber/double(burst_length), int(nbits));

    if (per <= per_target) break;
    else --mode;

  }

  return mode;
END_PROF("PHY::opt_mode")  
}

////////////////////////////////////////////////////////////////////////////////
// PHY::opt_power                                                             //
//                                                                            //
// returns optimal transmission power, i.e., such that the a packet with      //
// 'pack_len' data bytes and transmission mode 'mode' is received by        //
// terminal '*t1' with a packet error rate less than 'per_target'             //
////////////////////////////////////////////////////////////////////////////////
double PHY::opt_power(Terminal* t1, unsigned pack_len, double per_target,
                      transmission_mode mode, double pmin, double pmax,
                      double pstep) {
BEGIN_PROF("PHY::opt_power")
  double power = pmin;
  unsigned nbits = (DataMPDU(pack_len)).get_nbits();

  double att = ch->get_path_loss(t1->get_phy(), this) + NoiseVariance_dBm;
  
  for(;;) {
    double SNR = power - att;
  
    if (power >= pmax) break;

    double ber = calculate_ber(mode, SNR);
    double per = 1.0 - pow(1.0 - ber/double(burst_length), int(nbits));

    if (per <= per_target) break;
    else power += pstep;

  }

  return power;
END_PROF("PHY::opt_mode")  
}

////////////////////////////////////////////////////////////////////////////////
// PHY::receive                                                               //
//                                                                            //
// a packet 'p' is received with path loss 'path_loss' dB and interference    //
// level 'interf' mW. If packet is received correctly, forward it to MAC      //
// layer.                                                                     //
////////////////////////////////////////////////////////////////////////////////
void PHY::receive(MPDU pck, double path_loss, double interf) {
BEGIN_PROF("PHY::receive")

  double rx_pow = pck.get_power() - path_loss;

  if (rx_pow < CCASensitivity_dBm) {

    if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term << ": " 
                        << pck << " ignored, received power ( " << rx_pow
                        << "dBm) is below receiver sensitivity level" << endl;

  } else {
    // check if transceiver was busy
    timestamp duration = pck.get_duration();
    timestamp now = ptr2sch->now();

    if (((now - duration < busy_end) && (now - duration > busy_begin))
          || ((now < busy_end) && (now > busy_begin))) {

      if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term 
                          << " (PHY) : " << pck << " ignored"
                          << ", transceiver is busy" << endl;

      END_PROF("PHY::receive")
      return;
    }

    busy_end = now;
    busy_begin = now - duration;

    double NoiseInterfVar = (interf>0)? 10.0*log10(interf +
                                             pow(10.0,NoiseVariance_dBm/10.0))
                                      : NoiseVariance_dBm;

    double SNIR = rx_pow - NoiseInterfVar;
    double bit_error_prob = calculate_ber(pck.get_mode(), SNIR);


    double pack_error_prob = 1 - pow((1-bit_error_prob/burst_length),
                                     int(pck.get_nbits()));

    if (rand_gen->uniform() > pack_error_prob) {

      if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term 
                          << " (PHY) : SNIR = " << SNIR << ", PER = " 
                          << pack_error_prob << ", " << pck << " received " 
                          << endl;

      mymac->phyRxEndInd(pck);
    } else if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term 
                               << " (PHY) : SNIR = " << SNIR  << "dB, PER = " 
                               << pack_error_prob << ", " << pck 
                               << " not received " << endl;
  }
END_PROF("PHY::receive")
}

////////////////////////////////////////////////////////////////////////////////
// PHY::send                                                                  //
//                                                                            //
// send packet 'p' to wireless channel. If 'to_all' is true, then packet will //
// be received by all terminals in network. If 'to_all' is false, then only   //
// target terminal will receive the packet.                                   //
////////////////////////////////////////////////////////////////////////////////
void PHY::phyTxStartReq(MPDU p, bool to_all) {
BEGIN_PROF("PHY::phyTxStartReq")
       
  busy_begin = ptr2sch->now();
  busy_end = busy_begin + p.get_duration();

  if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term 
                      << " (PHY) : send " << p << " with power " 
                      << p.get_power() << "dBm" << endl;

  energy += double(p.get_duration()) * pow(10,p.get_power()/10);

  if (to_all) ch->send_packet_all(p);
  else ch->send_packet_one(p);

END_PROF("PHY::phyTxStartReq")
}

////////////////////////////////////////////////////////////////////////////////
// output operator <<                                                         //
////////////////////////////////////////////////////////////////////////////////
ostream& operator << (ostream& os, const PHY& p) {
    return os << *(p.term);
}


