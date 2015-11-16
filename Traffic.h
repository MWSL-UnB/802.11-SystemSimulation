#ifndef _Traffic_h
#define _Traffic_h 1

#include <utility>
#include <vector>

#include "random.h"
#include "Scheduler.h"
#include "log.h"

class Terminal;

////////////////////////////////////////////////////////////////////////////////
// enum arrival_time_type                                                     //
//                                                                            //
// distribution of packet arrival interval                                    //
////////////////////////////////////////////////////////////////////////////////
typedef enum {CONSTANT,
              EXP} arrival_time_type;

ostream& operator<< (ostream& os, const arrival_time_type& att);
istream& operator>> (istream& is, arrival_time_type& att);

////////////////////////////////////////////////////////////////////////////////
// class discrete_prob                                                        //
//                                                                            //
// specifies a discrete random variable x with probability distribution p_x   //
//                                                                            //
// Usage:                                                                     //
// - the default constructor creates a distribution p_x(x=1) = 1.             //
//   The constructor discrete_prob(i) creates a distribution p_x(x=i) = 1.    //
//   The distribution can also be given as a vector of probability pairs,     //
//   with each pair containing an integer and the corresponding probability.  //
// - a different distribution can be read using the input operator >>.        //
//   The input stream must be in the form:                                    //
//     x1(p1);x2(p2);x3(p3);....                                              //
//   without spaces in between. The probabilities must sum up to 1.           //
// - the function 'new_value' returns one integer from the probability        //
//   distribution, depending on the argument 'randval'. The returned value is //
//     y = F^-1(randval)                                                      //
//   where F^-1 is the inverse of the c.d.f.                                  //
// 
////////////////////////////////////////////////////////////////////////////////
class discrete_prob {
  vector<pair<int,double> > pl;

  static double epsilon;
  
public:
  discrete_prob() {pl.assign(1,make_pair(1, 1.0));}
  discrete_prob(int i) {pl.assign(1,make_pair(i, 1.0));}
  discrete_prob(vector<pair<int,double> >& p);
  
  int new_value(double randval) const;
  // returns one value from the discrete random variable. The value returned is
  // the inverse of the c.d.f. for the argument 'randval'.

  double mean() const;

  friend ostream& operator<< (ostream& os, const discrete_prob& uv);
  friend istream& operator>> (istream& is, discrete_prob& uv);
};

////////////////////////////////////////////////////////////////////////////////
// struct traffic_struct                                                      //
//                                                                            //
// traffic parameters                                                         //                                                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
struct traffic_struct {
  double datarate;                  // offered data rate in Mbps
  discrete_prob packlen;            // packet length probability
  arrival_time_type arrival_time;   // arrival time distribution

  traffic_struct(double dr, discrete_prob pl, arrival_time_type at)
                : datarate(dr), packlen(pl), arrival_time(at) {}
};

////////////////////////////////////////////////////////////////////////////////
// class Traffic                                                              //
////////////////////////////////////////////////////////////////////////////////
class Traffic {
  Scheduler* ptr2sch; // pointer to simulation scheduler
  random*    randgen; // pointer to random number generator
  log_file*  mylog;   // pointer to log file
  bool       logflag;

  unsigned long n_created_packs; // number of generated packets 

  Terminal* source;
  Terminal* target;
  
  double  data_rate;               // offered load per link in Mbps
  discrete_prob  packlength_prob;  // packet length probability distribution
  arrival_time_type  arrival_time; // arrival time distribution
  double  packs_per_sec;           // transmitted packets per second

  virtual void new_packet();
  // creates a new packet and schedules next one  

public:
  Traffic(Scheduler* s,          // pointer to simulation scheduler
          random* r,             // pointer to random number generator
          log_file* l,           // pointer to log file
          Terminal* from,        // source terminal
          Terminal* to,          // target terminal
          traffic_struct tr      // traffic parameters
          );

  static void wrapper_to_new_packet (void* ptr2obj) {
    ((Traffic*)ptr2obj)->new_packet();}
  // wrapper function to schedule next packet transmission

};
#endif

