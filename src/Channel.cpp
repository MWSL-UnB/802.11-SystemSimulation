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

#include <stdio.h>
#include <math.h>
#include <complex>
#include <algorithm>
#include <iostream>
//#include <stdlib.h>

#include "Channel.h"
#include "timestamp.h"
#include "long_integer.h"
#include "Profiler.h"
#include "mymath.h"
#include "myexception.h"
#include "PHY.h"
#include "Terminal.h"
#include "Standard.h"

// Channel model parameters
valarray<double> tapsPow_A{ 0.000000};
valarray<double> tapDelay_A{ 000000};

valarray<double> tapsPow_B{ 0.000000, -5.400000, -2.504133, -5.876886, -9.151515, -12.500000, -15.600000, -18.700000, -21.800000};
valarray<double> tapDelay_B{ 000000, 1.000000e-08, 2.000000e-08, 3.000000e-08, 4.000000e-08, 5.000000e-08, 6.000000e-08, 7.000000e-08, 8.000000e-08};

valarray<double> tapsPow_C{ 0.000000, -2.100000, -4.300000, -6.500000, -8.600000, -10.800000, -4.361080, -6.561080, -8.661080, -10.861080, -13.700000, -15.800000, -18.000000, -20.200000};
valarray<double> tapDelay_C{ 000000, 1.000000e-08, 2.000000e-08, 3.000000e-08, 4.000000e-08, 5.000000e-08, 6.000000e-08, 7.000000e-08, 8.000000e-08, 9.000000e-08, 1.100000e-07, 1.400000e-07, 1.700000e-07, 2.000000e-07};

valarray<double> tapsPow_D{ 0.000000, -0.900000, -1.700000, -2.600000, -3.500000, -4.300000, -5.200000, -6.100000, -6.900000, -7.800000, -4.625981, -7.216430, -9.816430, -12.416430, -13.652351, -17.950897, -22.337110, -26.700000};
valarray<double> tapDelay_D{ 000000, 1.000000e-08, 2.000000e-08, 3.000000e-08, 4.000000e-08, 5.000000e-08, 6.000000e-08, 7.000000e-08, 8.000000e-08, 9.000000e-08, 1.100000e-07, 1.400000e-07, 1.700000e-07, 2.000000e-07, 2.400000e-07, 2.900000e-07, 3.400000e-07, 3.900000e-07};

valarray<double> tapsPow_E{ -2.600000, -3.000000, -3.500000, -3.900000, 0.066829, -1.225981, -2.525981, -3.825981, -3.354724, -5.534855, -7.642636, -4.890378, -12.042636, -14.214719, -15.328471, -18.336614, -20.700000, -24.600000};
valarray<double> tapDelay_E{ 000000, 1.000000e-08, 2.000000e-08, 3.000000e-08, 5.000000e-08, 8.000000e-08, 1.100000e-07, 1.400000e-07, 1.800000e-07, 2.300000e-07, 2.800000e-07, 3.300000e-07, 3.800000e-07, 4.300000e-07, 4.900000e-07, 5.600000e-07, 6.400000e-07, 7.300000e-07};

valarray<double> tapsPow_F{ -3.300000, -3.600000, -3.900000, -4.200000, 0.032150, -0.862241, -1.633171, -2.533171, -1.453111, -2.941635, -4.316104, -5.863526, -5.242057, -7.863674, -9.353570, -13.164859, -16.300000, -21.200000};
valarray<double> tapDelay_F{ 000000, 1.000000e-08, 2.000000e-08, 3.000000e-08, 5.000000e-08, 8.000000e-08, 1.100000e-07, 1.400000e-07, 1.800000e-07, 2.300000e-07, 2.800000e-07, 3.300000e-07, 4.000000e-07, 4.900000e-07, 6.000000e-07, 7.300000e-07, 8.800000e-07, 1.050000e-06};

//////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////////
// auxiliary classes and functions                                            //
//////////////////////////////////////////////////////////////////////////////// 

//////////////////////////////////////////////////////////////////////////////// 
// output operator for valarray                                               //
////////////////////////////////////////////////////////////////////////////////
template <class T> ostream& operator << (ostream& os, const valarray<T>& x) {
  for (int i = 0; i < x.size() - 1; ++i) os << x[i] << ", ";
  os << x[x.size()-1];
  return os;
}


////////////////////////////////////////////////////////////////////////////////
// class recalc_interference                                                  //
//                                                                            //
// function-like object                                                       //
// recalculates interference when a new packet is added to channel.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
class recalc_interference {
  map<term_pair,double> path_loss;
  MPDU pack;
  double interf;

public:
  recalc_interference (
   const map<term_pair,double>& pl, // map with all possible links and path loss
   const MPDU& p                    // new packet added to channel
   ) : path_loss(pl), pack(p) {interf = 0;}

  void operator() (pack_struct& ps);
  // recalculates interference at 'ps'
  
  double get_interf() const {return interf;}
};

/////////////////////////////////////
// recalc_interference::operator() //
/////////////////////////////////////
void recalc_interference::operator() (pack_struct& ps) {
  MPDU interf_pack = ps.pck;
  if (pack.get_target() == interf_pack.get_target()) {
    interf = HUGE_VAL;
  } else {
    double pl = path_loss[term_pair((interf_pack.get_source())->get_phy(),
                          (pack.get_target())->get_phy())];
    interf += pow(10.0,(interf_pack.get_power() - pl)/10.0);
  }

  if (pack.get_source() == interf_pack.get_target()) {
    ps.interf_max = ps.interf = HUGE_VAL;
  } else {
    double pl = path_loss[term_pair((pack.get_source())->get_phy(),
                          (interf_pack.get_target())->get_phy())];
    ps.interf += pow(10.0,(pack.get_power() - pl)/10.0);
    if (ps.interf > ps.interf_max) {
      ps.interf_max = ps.interf;
    }
  }
};

/*
 * Jakes class
 *
 * Performs Jakes' method
 */
Jakes::Jakes() {
	doppler_spread = 0.0;
	cosbeta.resize(1,0.0);
	sinbeta.resize(1,0.0);
	omega.resize(1,0.0);
	theta.resize(1,0.0);
	cosalpha = 0.0;
	sinalpha = 0.0;
	n_osc = 0;
	xabs = 0.0;
};
Jakes::Jakes(double fd, unsigned no, random* r) {

	n_osc = no;

	doppler_spread = 2*M_PI*fd;

	theta.resize(n_osc);

	valarray<double> beta(M_PI/n_osc,n_osc);
	for (unsigned index = 0; index < n_osc; ++index) {
		beta[index] *= index+1.0;
		theta[index] = r->uniform(0,2*M_PI);
	}
	double alpha = r->uniform(0,2*M_PI);

	cosalpha = cos(alpha);
	sinalpha = sin(alpha);

	omega.resize(n_osc);
	omega = doppler_spread * cos(beta * double(n_osc) / double(2*n_osc+1));

	cosbeta.resize(n_osc);
	sinbeta.resize(n_osc);
	cosbeta = cos(beta);
	sinbeta = sin(beta);

	xabs = 0.0;

};

double Jakes::fade_calc(timestamp t) {

	  // calculate fading
	  double t_aux = double(t);

	  valarray<double> cosomegat = cos(omega*t_aux+theta);
	  valarray<double> aux1 = cosbeta * cosomegat;
	  valarray<double> aux2 = sinbeta * cosomegat;
	  complex<double> x(2*aux1.sum() + M_SQRT2*cosalpha*cos(doppler_spread*t_aux),
	                    2*aux2.sum() + M_SQRT2*sinalpha*cos(doppler_spread*t_aux));
	  x *= 1.0 / sqrt(n_osc + .5);

	  xabs = abs(x);

	  return xabs;
};

////////////////////////////////////////////////////////////////////////////////
// class same_link                                                            //
//                                                                            //
// function-like object                                                       //
// returns whether a terminal pair 't' is associated with a given link 'l'    //
////////////////////////////////////////////////////////////////////////////////
class same_link : public unary_function<Link, bool> {
  term_pair terms;
public:
  explicit same_link(const term_pair& t ) : terms(t) {};
  bool operator()(const Link& l) const {return l.belong(terms);}
};


////////////////////////////////////////////////////////////////////////////////
// class same_packet                                                          //
//                                                                            //
// function-like object                                                       //
// returns whether 'id' is the identification number of packet 'p'            //
////////////////////////////////////////////////////////////////////////////////
class same_packet : public unary_function<pack_struct, bool> {
  long_integer id;
public:
  inline explicit same_packet(long_integer i) : id(i) {};
  inline bool operator()(const pack_struct& p) const {
    return (p.pck).get_id() == id;}
};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class term_pair                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// term_pair constructor                                                      //
////////////////////////////////////////////////////////////////////////////////
term_pair::term_pair(PHY* t1, PHY* t2){
    tp = (t1->get_id() <= t2->get_id())? tp_type(t1,t2) : tp_type(t2,t1);
}

////////////////////////////////////////////////////////////////////////////////
// termpair::operator <                                                       //
////////////////////////////////////////////////////////////////////////////////
inline bool term_pair::operator< (const term_pair& p) const {
  if ((tp.first)->get_id() < (p.tp.first)->get_id()) return true;
  if ((tp.first)->get_id() == (p.tp.first)->get_id())
    if ((tp.second)->get_id() < (p.tp.second)->get_id()) return true;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// term_pair::operator==                                                      //
////////////////////////////////////////////////////////////////////////////////
inline bool term_pair::operator== (const term_pair& p) const {
  return (tp.first == p.tp.first && tp.second == p.tp.second);
}

////////////////////////////////////////////////////////////////////////////////
// output operator <<                                                         //
////////////////////////////////////////////////////////////////////////////////
ostream& operator << (ostream& os, const term_pair& t) {
  return os << *(t.tp.first) << " and " << *(t.tp.second);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Channel                                                              //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Channel constructor                                                        //
////////////////////////////////////////////////////////////////////////////////
Channel::Channel(Scheduler *s, random *r, channel_struct p, log_file *l){

  ptr2sch = s;
  rand_gen = r;
  
  mylog = l;
  logflag = (*mylog)(log_type::channel);

  LossExponent = p.loss_exponent;
  RefLoss_dB = p.ref_loss;
  DopplerSpread_Hz = p.doppler_spread;
  NumberSinus = p.number_sines;
  cModel = p.model;
}

////////////////////////////////////////////////////////////////////////////////
// Channel_private::busy_channel_message                                      //
//                                                                            //
// tell all the terminals requesting notification that channel was occupied   //
// by packet 'pck'                                                            //
////////////////////////////////////////////////////////////////////////////////
void Channel_private::busy_channel_message(MPDU pck) {
BEGIN_PROF("Channel::busy_channel_message")

  list<PHY*>::iterator it = waiting_list_busy.begin();
  
  double power = pck.get_power();
  PHY* source = (pck.get_source())->get_phy();

  while (it != waiting_list_busy.end()) {
    // iterator has to be stored in auxiliary variable 'it_aux' because it can
    // be deleted by channel_is_busy()
    list<PHY*>::iterator it_aux = it++;

    (*it_aux)->channel_occupied(get_interf_dBm(*it_aux));

  }

END_PROF("Channel::busy_channel_message")
}


////////////////////////////////////////////////////////////////////////////////
// Channel_private::free_channel_message                                      //
//                                                                            //
// tell all the terminals requesting notification that channel was released   //
// by packet 'pck'                                                            //
////////////////////////////////////////////////////////////////////////////////
void Channel_private::free_channel_message(MPDU pck) {
BEGIN_PROF("Channel::free_channel_message")

  list<PHY*>::iterator it = waiting_list_free.begin();
  while (it != waiting_list_free.end()) {
    // iterator has to be stored in auxiliary variable because it can be
    // deleted by channel_is_free()
    list<PHY*>::iterator it_aux = it++;
    (*it_aux)->channel_released(get_interf_dBm(*it_aux));
  }

END_PROF("Channel::free_channel_message")
}

////////////////////////////////////////////////////////////////////////////////
// Channel::get_interf_dBm                                                    //
//                                                                            //
// returns interference level at a given PHY                                  //
////////////////////////////////////////////////////////////////////////////////
double Channel::get_interf_dBm(PHY* t) {
BEGIN_PROF("Channel::get_interf_dBm")

  double max_interf = -HUGE_VAL;
  for (list<pack_struct>::const_iterator it = air_pack.begin();
       it != air_pack.end(); ++it) {
    double temp_interf = (it->pck).get_power()-
                         path_loss[term_pair(((it->pck).get_source())->get_phy()
                                             ,t)];
    if (temp_interf >= max_interf) max_interf = temp_interf;
  }

  return max_interf;
}


////////////////////////////////////////////////////////////////////////////////
// Channel::get_path_loss                                                     //
//                                                                            //
// returns path loss in dB between two given PHYs                             //
////////////////////////////////////////////////////////////////////////////////
double Channel::get_path_loss (PHY *t1, PHY *t2) {
  return path_loss[term_pair(t1,t2)];
}

////////////////////////////////////////////////////////////////////////////////
// Channel::get_channel_model                                                 //
//                                                                            //
// returns current channel model					                          //
////////////////////////////////////////////////////////////////////////////////
channel_model Channel::get_channel_model() {
	return cModel;
}


////////////////////////////////////////////////////////////////////////////////
// Channel::new_link                                                          //
//                                                                            //
// creates an active time-variant channel link between two terminals          //
////////////////////////////////////////////////////////////////////////////////
void Channel::new_link(PHY* pt1, PHY* pt2) {

  // if terminals are unknown to channel, add them to terminal list
  if (find(term_list.begin(), term_list.end(), pt1) == term_list.end()) {
    new_term(pt1);
  }
  if (find(term_list.begin(), term_list.end(), pt2) == term_list.end()) {
    new_term(pt2);
  }

  // create link if it doesn't exist yet
  term_pair tp(pt1,pt2);

  for(vector<Link>::const_iterator it = links.begin(); it != links.end(); ++it){
    if (it->belong(tp)) return;
  }
  
  Link newlink(tp, path_loss[tp], DopplerSpread_Hz, rand_gen, NumberSinus, cModel);
  links.push_back(newlink);
  path_loss[tp] = newlink.fade(ptr2sch->now());

  if (logflag) *mylog << "Channel: New time-variant link created between "
                      << *pt1 << " and " << *pt2 << ", path_loss = " 
                      << path_loss[tp] << "dB" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// Channel_private::new_term                                                  //
//                                                                            //
// adds new terminal to terminal list.                                        //
////////////////////////////////////////////////////////////////////////////////
void Channel_private::new_term(PHY* t) {

  if (logflag) *mylog << "Channel: " << *t << " added to channel" << endl;

  Position pos = t->get_pos();

  for(vector<PHY*>::const_iterator it = term_list.begin();
      it != term_list.end(); ++it) {
    double distance = pos.distance((*it)->get_pos());
    double pl = RefLoss_dB + 10.0*LossExponent*log10(distance);
    path_loss[term_pair(*it,t)] = pl;

    if (logflag) *mylog << "\tpath loss between " << *t << " and " << **it 
                        << " is " << pl << "dB" << endl;
  }

  term_list.push_back(t);
}


////////////////////////////////////////////////////////////////////////////////
// Channel::send_packet_all                                                   //
//                                                                            //
// sends packet to be received by all terminals.                              //
////////////////////////////////////////////////////////////////////////////////
void Channel::send_packet_all(MPDU pack) {

BEGIN_PROF("Channel::send_packet_all")

  // check for existing packets and calculate interference
  if (air_pack.size()) {
    recalc_interference interf(path_loss, pack);
    interf = for_each(air_pack.begin(), air_pack.end(), interf);

    air_pack.push_back(pack_struct(pack,interf.get_interf()));
  } else {
    air_pack.push_back(pack_struct(pack));
  }

  // schedule transmission end
  ptr2sch->schedule(Event(ptr2sch->now() + pack.get_duration(),
                    (void*)&wrapper_to_stop_send_all,
                    (void*)this, pack.get_id()));

  // update channel gain
  if (DopplerSpread_Hz > 0) {
    term_pair tp((pack.get_source())->get_phy(),(pack.get_target())->get_phy());
    vector<Link>::iterator itl = find_if(links.begin(), links.end(),
                                         same_link(tp));
    path_loss[tp] = itl->fade(ptr2sch->now());
  }

  busy_channel_message (pack);
END_PROF("Channel::send_packet_all")

}

////////////////////////////////////////////////////////////////////////////////
// Channel::send_packet_one                                                   //
//                                                                            //
// sends packet to be received only by target terminal.                       //
////////////////////////////////////////////////////////////////////////////////
void Channel::send_packet_one(MPDU pack) {

BEGIN_PROF("Channel::send_packet_one")

  if (air_pack.size()) {
    // check for existing packets and calculate interference
    recalc_interference interf(path_loss, pack);
    interf = for_each(air_pack.begin(), air_pack.end(), interf);

    air_pack.push_back(pack_struct(pack,interf.get_interf()));
  } else {
    air_pack.push_back(pack_struct(pack));
  }

  // schedule transmission end
  ptr2sch->schedule(Event(ptr2sch->now() + pack.get_duration(),
                    (void*)&wrapper_to_stop_send_one,
                    (void*)this, pack.get_id()));

    if (DopplerSpread_Hz > 0) {
      term_pair tp((pack.get_source())->get_phy()
                   ,(pack.get_target())->get_phy());
      vector<Link>::iterator itl = find_if(links.begin(), links.end(),
                                          same_link(tp));
      path_loss[tp] = itl->fade(ptr2sch->now());
    }

  busy_channel_message (pack);

END_PROF("Channel::send_packet_one")

}

////////////////////////////////////////////////////////////////////////////////
// Channel_private::stop_send_all                                             //
//                                                                            //
// stops packet transmission over the channel. All terminals receive it.      //
////////////////////////////////////////////////////////////////////////////////
void Channel_private::stop_send_all(long_integer pack_id) {
BEGIN_PROF("Channel::stop_send_all")

  list<pack_struct>::iterator it = find_if(air_pack.begin(), air_pack.end(),
                                           same_packet(pack_id));

  if (it == air_pack.end())
    throw(GENERAL,"Packet not found in Channel::stop_send_all");

  MPDU pack = it->pck;

  PHY* source = (pack.get_source())->get_phy();
  PHY* target = (pack.get_target())->get_phy();

  // send to target terminal
  valarray<double> pLoss(path_loss[term_pair(source,target)],Standard::get_numSubcarriers());
  target->receive(pack, pLoss,it->interf_max);

  air_pack.erase(it);


  // send to all other terminals
  for (vector<PHY*>::iterator term_it = term_list.begin();
       term_it != term_list.end(); ++term_it) {
    if (*term_it != target && *term_it != source) {

      valarray<double> pLoss(path_loss[term_pair(source,*term_it)],Standard::get_numSubcarriers());
      (*term_it)->receive(pack, pLoss);
    }
  }

  free_channel_message(pack);

  // recalculate interference at active packets
  double power_dB = pack.get_power();
  for (it = air_pack.begin(); it != air_pack.end(); ++it) {
    it->interf -= pow(10.0,
                      (power_dB
                       - path_loss[term_pair(source,
                                 ((it->pck).get_target())->get_phy())]) / 10.0);
  }
END_PROF("Channel::stop_send_all")

}

////////////////////////////////////////////////////////////////////////////////
// Channel_private::stop_send_one                                             //
//                                                                            //
// stops packet transmission over the channel.                                //
// Only target terminal receive it.                                           //
////////////////////////////////////////////////////////////////////////////////
void Channel_private::stop_send_one(long_integer pack_id) {
BEGIN_PROF("Channel::stop_send_one")

  list<pack_struct>::iterator it = air_pack.begin();
  it = find_if(air_pack.begin(),air_pack.end(),same_packet(pack_id));

  if (it == air_pack.end())
    throw(GENERAL,"Packet not found in Channel::stop_send_all");

  MPDU pack = it->pck;

  PHY* source = (pack.get_source())->get_phy();
  PHY* target = (pack.get_target())->get_phy();

  valarray<double> pLoss(path_loss[term_pair(source,target)],Standard::get_numSubcarriers());
  target->receive(pack, pLoss,it->interf_max);

  air_pack.erase(it);
  free_channel_message(pack);

  // recalculate interference at active packets
  double power_dB = pack.get_power();
  for (it = air_pack.begin(); it != air_pack.end(); ++it) {
    it->interf -= pow(10.0
                      ,(power_dB
                       - path_loss[term_pair((pack.get_source())->get_phy()
                                              ,(pack.get_target())->get_phy())])
                       / 10.0);
  }
END_PROF("Channel::stop_send_one")

}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Link                                                                 //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Link constructor                                                           //
////////////////////////////////////////////////////////////////////////////////
Link::Link(term_pair t, double pl, double fd, random* r, unsigned ns, channel_model cm)
: terms(t), path_loss_mean(pl) {

	time_last = timestamp(0);
	time_diff_min = -1;
	doppler_spread = 2*M_PI*fd;

	switch(cm) {
	case A:
		nTaps = tapsPow_A.size();
		taps_amps = from_dB(tapsPow_A);
		taps_delays = tapDelay_A;
		break;
	case B:
		nTaps = tapsPow_B.size();
		taps_amps = from_dB(tapsPow_B);
		taps_delays = tapDelay_B;
		break;
	case C:
		nTaps = tapsPow_C.size();
		taps_amps = from_dB(tapsPow_C);
		taps_delays = tapDelay_C;
		break;
	case D:
		nTaps = tapsPow_D.size();
		taps_amps = from_dB(tapsPow_D);
		taps_delays = tapDelay_D;
		break;
	case E:
		nTaps = tapsPow_E.size();
		taps_amps = from_dB(tapsPow_E);
		taps_delays = tapDelay_E;
		break;
	case F:
		nTaps = tapsPow_F.size();
		taps_amps = from_dB(tapsPow_F);
		taps_delays = tapDelay_F;
		break;
	}

	taps_amps_fade.resize(nTaps,0.0);

	taps_jks.clear();
	for(unsigned k = 0; k < nTaps; ++k){
		Jakes auxJks = Jakes(fd,ns,r);
		taps_amps_fade[k] = taps_amps[k]*auxJks.fade_calc(timestamp(0));
		taps_jks.push_back(auxJks);
	}

	cout << "Taps amps: 		";
	for(unsigned k = 0; k < nTaps; ++k){
		cout << " " << taps_amps[k];
	}
	cout << endl;

	cout << "Taps amps fade: 	";
	for(unsigned k = 0; k < nTaps; ++k){
		cout << " " << taps_amps_fade[k];
	}
	cout << endl;

	resample();

	// Multiply by 2 since x is an amplitude value (20log10 instead of 10log10)
	path_loss = path_loss_mean - 2*to_dB(taps_jks[0].fade_calc(timestamp(0)));

}

////////////////////////////////////////////////////////////////////////////////
// Link::fade                                                                 //
//                                                                            //
// updates channel fading,                                                    //
// and returns the link gain amplitude at time 't' in dB                      //
////////////////////////////////////////////////////////////////////////////////
double Link::fade(timestamp t) {
BEGIN_PROF("Link::fade")

  double time_diff_new = double(t - time_last);

  if (time_diff_new <= time_diff_min) {
	  return path_loss;
  }

			  // if correlation is too large, channel does not change
  if (bessel_j0(doppler_spread * time_diff_new) >= .9999) {
	time_diff_min = time_diff_new;
	return path_loss;
  }

  for(unsigned k = 0; k < nTaps; ++k){
	  taps_amps_fade[k] = taps_amps[k]*taps_jks[k].fade_calc(timestamp(t));
  }


  // Multiply by 2 since x is an amplitude value (20log10 instead of 10log10)
  path_loss = path_loss_mean - 2*to_dB(taps_jks[0].fade_calc(t));


#ifdef _SAVE_RATE_ADAPT
  rate_adapt_file_ch << setw(10) << t_aux << ','
                     << setw(6) << (terms.get_ids()).first << ','
                     << setw(6) << (terms.get_ids()).second << ','
                     << setw(10) << path_loss << ','
                     << setw(10) << x.real() << ','
                     << setw(10) << x.imag() << endl;
#endif

END_PROF("Link::fade")
  return path_loss;
}

valarray<double> Link::resample() {

	double W = Standard::get_band_double();
	double sample_time = 1/W;
	unsigned max_samp = (unsigned)ceil(taps_delays[nTaps - 1]/sample_time);

	valarray<double> samples;
	samples.resize(max_samp + 1,0.0);
	double time = 0.0;
	for(unsigned k = 0; k <= max_samp; ++k) {
		time = k*sample_time;
		double time_diff;
		for(unsigned j = 0; j < nTaps; j++){
			time_diff = time - taps_delays[j];
			samples[k] += taps_amps_fade[j]*invraisedcos(time_diff,W,Standard::get_rollof());
		}

	}

	cout << "Samples: 		";
	for(unsigned k = 0; k <= max_samp; ++k) {
		cout << " " << samples[k];
	}
	cout << endl;

	return samples;

}

////////////////////////////////////////////////////////////////////////////////
// output operator <<                                                         //
////////////////////////////////////////////////////////////////////////////////
ostream& operator << (ostream& os, const Link& l) {
 return os << "link between " << l.terms << ", path loss = "
           << l.path_loss << " dB";
}

////////////////////////////////////////////////////////////////////////////////


////////////////////////
// output operator<<  //
////////////////////////
ostream& operator<< (ostream& os, const channel_model& cm) {
  switch(cm) {
    case A : return os << "A";
    case B : return os << "B";
    case C : return os << "C";
    case D : return os << "D";
    case E : return os << "E";
    case F : return os << "F";
    default: return os << "unknown Model.";
  }
}

///////////////////////
// input operator >> //
///////////////////////
istream& operator>> (istream& is, channel_model& cm) {
  string str;
  is >> str;

  if (str == "A") cm = A;
  else if (str == "B") cm = B;
  else if (str == "C") cm = C;
  else if (str == "D") cm = D;
  else if (str == "E") cm = E;
  else if (str == "F") cm = F;
  else is.clear(ios::failbit);

  return is;
}
