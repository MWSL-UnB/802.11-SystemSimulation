#include "Traffic.h"

#include "Terminal.h"
#include "Packet.h"
#include "myexception.h"
#include "Profiler.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// enum arrival_time_type                                                     //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// output operator<< arrival_time_type                                        //
////////////////////////////////////////////////////////////////////////////////
ostream& operator<< (ostream& os, const arrival_time_type& att) {
  switch (att) {
    case CONSTANT : return os << "constant";
    case EXP      : return os << "exponential";
  }
}

////////////////////////////////////////////////////////////////////////////////
// input operator>> arrival_time_type                                         //
////////////////////////////////////////////////////////////////////////////////
istream& operator>> (istream& is, arrival_time_type& att) {
  string str;
  is >> str;

  if (str == "CONST" || str == "CONSTANT") att = CONSTANT;
  else if (str == "EXP" || str == "EXPONENTIAL") att = EXP;
  else is.clear(ios::failbit);

  return is;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class discrete_prob                                                        //
////////////////////////////////////////////////////////////////////////////////
double discrete_prob::epsilon = 1.0e-3;

////////////////////////////////////////////////////////////////////////////////
// discrete_prob constructor                                                  //
////////////////////////////////////////////////////////////////////////////////
discrete_prob::discrete_prob(vector<pair<int,double> >& p) {
  pl = p;
  
  double total_prob = 0;
  for (vector<pair<int,double> >::const_iterator it = pl.begin();
       it != pl.end(); ++it) total_prob += it->second;
       
  if (fabs(total_prob - 1.0) > epsilon) 
    throw(my_exception(GENERAL,
                "discrete_prob constructor: probability does not add up to 1"));
}

////////////////////////////////////////////////////////////////////////////////
// input operator>> discrete_prob                                             //
////////////////////////////////////////////////////////////////////////////////
istream& operator>> (istream& is, discrete_prob& uv) {

  string::size_type pos;
  uv.pl.clear();

  int count = 0;
  double total_prob = 0;

  char c;
  do {

    ++count;

    int length;
    is >> length;

    if (is.fail()) return is;

    is >> c;

    if (c != '(') {
      is.putback(c);
      is.clear();

      if (count == 1) {
       uv.pl.push_back(make_pair(length,1.0));
       return is;
      } else {
        break;
      }
    }

    double prob;
    if ((is >> prob >> c).fail()) return is;

    if (c != ')') break;

    total_prob += prob;

    uv.pl.push_back(make_pair(length,total_prob));

  } while (is >> c, c == ';');

  if (count != uv.pl.size()) {
    is.clear(ios::failbit);
  }

  if (fabs(total_prob - 1.0) > discrete_prob::epsilon) is.clear(ios::failbit);

  is.clear();
  return is;
}

////////////////////////////////////////////////////////////////////////////////
// output operator<< discrete_prob                                            //
////////////////////////////////////////////////////////////////////////////////
ostream& operator<< (ostream& os, const discrete_prob& uv) {
  double previous = 0;
  for (vector<pair<int,double> >::const_iterator it = uv.pl.begin();
       it != uv.pl.end(); ++it) {
    if (it != uv.pl.begin()) os << ";";
    os << it->first << "(" << it->second - previous << ")";
    previous = it->second;
  }

  return os;
}

////////////////////////////////////////////////////////////////////////////////
// discrete_prob::mean                                                        //
////////////////////////////////////////////////////////////////////////////////
double discrete_prob::mean() const {
  double m = 0;
  for (vector<pair<int,double> >::const_iterator it = pl.begin();
       it != pl.end(); ++it) {
    m += double(it->first) * it->second;
  }

  return m;
}

////////////////////////////////////////////////////////////////////////////////
// discrete_prob::new_value                                                   //
//                                                                            //
// returns one value from the discrete random variable. The value returned is //
// the inverse of the c.d.f. for the argument 'randval'.                      //
////////////////////////////////////////////////////////////////////////////////
int discrete_prob::new_value(double randval) const {
  double p;
  for (vector<pair<int,double> >::const_iterator it = pl.begin();
       it != pl.end(); ++it) {
    if (randval <= it->second) return it->first;
  }
  return (pl.rbegin())->first;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Traffic                                                              //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Traffic constructor                                                       //
////////////////////////////////////////////////////////////////////////////////
Traffic::Traffic(Scheduler* s, random* r, log_file* l,
                 Terminal* from, Terminal* to, traffic_struct tr) {

  ptr2sch = s;
  randgen = r;

  mylog = l;
  logflag = (*mylog)(log_type::traffic);
  
  source = from;
  target = to;
  
  data_rate = tr.datarate;
  packlength_prob = tr.packlen;
  arrival_time = tr.arrival_time;
  
  n_created_packs = 0;
  
  packs_per_sec = data_rate/(packlength_prob.mean() * 8.0);

  // Generate first packet
  if (packs_per_sec > 0) {

    timestamp time_arrival;
    if (arrival_time == CONSTANT) {
      time_arrival = timestamp(randgen->uniform(0,1.0/packs_per_sec));
    } else {
      time_arrival = timestamp(randgen->exponential(packs_per_sec));
    }

    if (logflag) *mylog << *source << " generates first packet at " 
                        << time_arrival << "secs" << endl;

    ptr2sch->schedule(Event(time_arrival, (void*)(&wrapper_to_new_packet),
                            (void*)this));
  }
  
}

////////////////////////////////////////////////////////////////////////////////
// Traffic::new_packet                                                        //
//                                                                            //
// creates a new packet and schedules next one                                //
////////////////////////////////////////////////////////////////////////////////
void Traffic::new_packet() {
BEGIN_PROF("Traffic::new_packet")

  ++n_created_packs;

  timestamp time_arrival = ptr2sch->now();
  MSDU pck(packlength_prob.new_value(randgen->uniform()), source, target, 0,
           time_arrival);

  if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *source 
                      << " generates a new packet for " << *target << " with " 
                      << pck.get_nbytes() << " data bytes" << endl;
           
  source->macUnitdataReq(pck);          
  
  switch (arrival_time) {
    case EXP: time_arrival += timestamp(randgen->exponential(packs_per_sec));
              break;
    case CONSTANT: time_arrival += timestamp(1.0/packs_per_sec);
                   break;
  }

  ptr2sch->schedule(Event(time_arrival, (void*)(&wrapper_to_new_packet),
                          (void*)this));

END_PROF("Traffic::new_packet")
}


