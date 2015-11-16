#ifndef _link_adapt_private_h
#define _link_adapt_private_h 1

#include "Packet.h"
#include "log.h"

class Terminal;

////////////////////////////////////////////////////////////////////////////////
// enum adapt_mode                                                            //
//                                                                            //
// what is adapted                                                            //
////////////////////////////////////////////////////////////////////////////////
typedef enum {POWER, RATE} adapt_mode;

ostream& operator<< (ostream& os, const adapt_mode& tm);
istream& operator>> (istream& is, adapt_mode& tm);


////////////////////////////////////////////////////////////////////////////////
// class link_adapt_private                                                   //
//                                                                            //
// declares private member objects and functions of class link_adapt          //
////////////////////////////////////////////////////////////////////////////////
class link_adapt_private {
protected:
  Terminal* source;  // link between terminals *source
  Terminal* target;  // and *target

  log_file*  mylog;
  bool       logflag;  // true if link adaptation events should be logged

  transmission_mode mode; // rate adaptation strategy

  adapt_mode adapt;
  
  double target_per;  // PER not to be exceeded if "mode == OPT"
  
  double power_dBm;  // transmit power in dBm
  double pmax, pmin, pstep_u, pstep_d, pstep_min;
  

  /////////////////////////////////////////////////
  // link adaptation parameters if "mode == SUBOPT"
  unsigned max_succeed_counter; // increase data rate when 'max_succeed_counter'
                                // packets are successfully transmitted
  unsigned fail_limit; // reduce data rate when 'fail_limit' packets cannot be
                       // transmitted
  bool use_rx_mode; // true if data rate of received packets is used to adapt
                    // transmission rate   
  bool adapt_la_thresh; // true if adaptation threshold 'max_succeed_counter'
                        // should be adapted

  unsigned succeed_counter; // number of successively transmitted packets
  unsigned fail_counter;    // number of successively failed transmissions

  transmission_mode current_mode; // current optimised transmission rate
  
  void adapt_opt(unsigned pl);
};

#endif
