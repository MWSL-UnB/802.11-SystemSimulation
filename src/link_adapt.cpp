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

#include "link_adapt.h"
#include "Terminal.h"
#include "Profiler.h"

unsigned LA_max_success_counter_LOW = 10;
unsigned LA_max_success_counter_HIGH = 3;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// enum adapt_mode                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////
// output operator << //
////////////////////////
ostream& operator<< (ostream& os, const adapt_mode& tm) {
  switch(tm) {
    case POWER: return os << "POWER";
    case RATE: return os << "RATE";
  }
}

///////////////////////
// input operator >> //
///////////////////////
istream& operator>> (istream& is, adapt_mode& tm) {
  string str;
  is >> str;

  if (str == "POWER") tm = POWER;
  else if (str == "RATE") tm = RATE;
  else is.clear(ios::failbit);

  return is;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class link_adapt                                                           //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// constructor                                                                //
////////////////////////////////////////////////////////////////////////////////
link_adapt::link_adapt (Terminal* from, Terminal* to, adapt_struct param,
                        log_file* l) {

  source = from;
  target = to;

  mylog = l;
  logflag = (*mylog)(log_type::adapt);

  target_per = param.per;
  power_dBm = pmax = param.power_max;
  pmin = param.power_min;
  pstep_d = param.power_step_down;
  pstep_u = param.power_step_up;
  pstep_min = (pstep_d < pstep_u)? pstep_d : pstep_u;
  
  adapt = param.adapt;

  mode = param.mode;
  if (mode == SUBOPT) {
    max_succeed_counter = param.max_succeed_counter;
    if (!max_succeed_counter) {
      max_succeed_counter = LA_max_success_counter_HIGH;
      adapt_la_thresh = true;
    } else {
      adapt_la_thresh = false;
    }
    fail_limit = param.la_fail_limit;
    use_rx_mode = param.use_rx_mode;

    current_mode = M6;
    succeed_counter = 0;
    fail_counter = 0;
  } else {
    current_mode = mode;
  }
}

////////////////////////////////////////////////////////////////////////////////
// link_adapt_private::adapt_opt                                              //
////////////////////////////////////////////////////////////////////////////////
void link_adapt_private::adapt_opt(unsigned pl) {
 if (adapt == RATE) {
   current_mode = (target->get_phy())->opt_mode(source, pl, target_per,
                                                power_dBm);
 } else {
   current_mode = M6;
   power_dBm = (target->get_phy())->opt_power(source, pl, target_per,
                                              current_mode, pmin, pmax,
                                              pstep_min);
   if (power_dBm > pmax) power_dBm = pmax;
   if (power_dBm < pmin) {
     power_dBm = pmin;
     current_mode = (target->get_phy())->opt_mode(source, pl, target_per,
                                                  power_dBm);
   }
 }
}

////////////////////////////////////////////////////////////////////////////////
// link_adapt::get_current_mode                                               //
////////////////////////////////////////////////////////////////////////////////
transmission_mode link_adapt::get_current_mode(unsigned pl) {
  if (mode == OPT) adapt_opt(pl);
  
  return current_mode;
}

////////////////////////////////////////////////////////////////////////////////
// link_adapt::get_power                                                      //
////////////////////////////////////////////////////////////////////////////////
double link_adapt::get_power(unsigned pl) {
  if (mode == OPT) adapt_opt(pl);
  
  return power_dBm;
}

////////////////////////////////////////////////////////////////////////////////
// link_adapt::failed                                                         //
//                                                                            //
// ACK timed out, transmission failed, adapt link                             //
////////////////////////////////////////////////////////////////////////////////
void link_adapt::failed () {
BEGIN_PROF("link_adapt::failed")

  if (mode != SUBOPT) return;

  ++fail_counter;
  succeed_counter = 0;

  if (logflag) *mylog << "\n" << source->time() << "sec., " << *source 
                      << ": fragment transmission failed"
                      << ", update fail counter to " << fail_counter
                      << ", reset succeed counter" << endl;

  if (!max_succeed_counter) {
    max_succeed_counter = LA_max_success_counter_LOW;

    if (logflag) *mylog << "    Terminal is in adaptive threshold enquiry mode"
                        << ", set succeed counter threshold to low mobility ("
                        << max_succeed_counter << ")" << endl;

  }
  else if (adapt_la_thresh) {
    max_succeed_counter = LA_max_success_counter_HIGH;

    if (logflag) *mylog << "    adaptive threshold set to high mobility ("
                        << max_succeed_counter << ")" << endl;
  }

  if (fail_counter >= fail_limit) {
    fail_counter = 0;
    succeed_counter = 0;
    
    if (adapt == RATE || current_mode > M6) {
      --current_mode;

      if (logflag) *mylog << "    fail counter = maximum fail counter = "
                          << fail_limit << ", decrease tx rate to "
                          << current_mode << endl;
    } else {
      if (power_dBm + pstep_u <= pmax) power_dBm += pstep_u;

      if (logflag) *mylog << "    fail counter = maximum fail counter = "
                          << fail_limit << ", increase transmit power to "
                          << power_dBm << endl;
      
    }
  }
END_PROF("link_adapt::failed")
}


////////////////////////////////////////////////////////////////////////////////
// link_adapt::rts_failed                                                     //
//                                                                            //
// CTS timed out, transmission failed, adapt link                             //
////////////////////////////////////////////////////////////////////////////////
void link_adapt::rts_failed () {
BEGIN_PROF("link_adapt::rts_failed")

  if (mode != SUBOPT || adapt != POWER) return;

  ++fail_counter;
  succeed_counter = 0;

  if (logflag) *mylog << "\n" << source->time() << "sec., " << *source 
                      << ": RTS transmission failed"
                      << ", update fail counter to " << fail_counter
                      << ", reset succeed counter" << endl;

  if (!max_succeed_counter) {
    max_succeed_counter = LA_max_success_counter_LOW;

    if (logflag) *mylog << "    Terminal is in adaptive threshold enquiry mode"
                        << ", set succeed counter threshold to low mobility ("
                        << max_succeed_counter << ")" << endl;

  }
  else if (adapt_la_thresh) {
    max_succeed_counter = LA_max_success_counter_HIGH;

    if (logflag) *mylog << "    adaptive threshold set to high mobility ("
                        << max_succeed_counter << ")" << endl;
  }

  if (fail_counter >= fail_limit) {
    fail_counter = 0;
    succeed_counter = 0;
    
    if (current_mode > M6) {
      --current_mode;

      if (logflag) *mylog << "    fail counter = maximum fail counter = "
                          << fail_limit << ", decrease tx rate to "
                          << current_mode << endl;
    } else {
      if (power_dBm + pstep_u <= pmax) power_dBm += pstep_u;

      if (logflag) *mylog << "    fail counter = maximum fail counter = "
                          << fail_limit << ", increase transmit power to "
                          << power_dBm << endl;
      
    }
  }
END_PROF("link_adapt::rts_failed")
}

////////////////////////////////////////////////////////////////////////////////
// link_adapt::rx_success                                                     //
//                                                                            //
// data packet succesfully received, adapt link                               //
////////////////////////////////////////////////////////////////////////////////
void link_adapt::rx_success(transmission_mode rx_mode) {
BEGIN_PROF("link_adapt::rx_success")

  if (mode != SUBOPT || !use_rx_mode) return;

  if (logflag) *mylog << "\n" << source->time() << "sec., " << *source 
                      << ": link adaptation using rx packet" << endl;

  if (rx_mode > current_mode) {

    if (logflag) *mylog << "    received data rate (" << rx_mode
                        << ") > current data rate (" << current_mode
                        << "), increase it" << endl;

    succeed_counter = 0;
    fail_counter = 0;
    current_mode = rx_mode;


  }
END_PROF("link_adapt::rx_success")
}

////////////////////////////////////////////////////////////////////////////////
// link_adapt::success                                                        //
//                                                                            //
// ACK received, transmission succeeded, adapt link                           //
////////////////////////////////////////////////////////////////////////////////
void link_adapt::success(bool lastfrag) {
BEGIN_PROF("link_adapt::success")

  if (mode != SUBOPT) return;
  succeed_counter++;
  fail_counter = 0;

  if (logflag) {
    if (lastfrag) *mylog << "\n" << source->time() << "sec., " << *source
                         << ": last fragment transmitted correctly" << endl;
    else *mylog << "  " << *source
                  << ": intermediate fragment transmitted correctly" << endl;
    *mylog << "    succeed_counter = " << succeed_counter
             << ", maximum = " << max_succeed_counter << endl;
  }

  if (!max_succeed_counter) {
    max_succeed_counter = LA_max_success_counter_HIGH;

    if (logflag) *mylog << "    Terminal is in adaptive threshold enquiry mode"
                        << ", set succeed counter threshold to high mobility ("
                        << max_succeed_counter << ")" << endl;
  }

  if (succeed_counter >= max_succeed_counter && lastfrag) {
    succeed_counter = 0;
    
    if (adapt == RATE || power_dBm - pstep_d < pmin) {
      ++current_mode;

      if (logflag) *mylog << "    increase tx rate to " << current_mode << endl;
    } else {
      power_dBm -= pstep_d;

      if (logflag) *mylog << "    decrease tx power to " << power_dBm << endl;
      
    }

    if (adapt_la_thresh) {
      max_succeed_counter = 0;

      if (logflag) *mylog << "    enter adaptive threshold enquiry mode" << endl;
    }
  }

END_PROF("link_adapt::success")
}

