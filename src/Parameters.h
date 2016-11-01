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

#ifndef _parameters_h
#define _parameters_h 1

#include <string>
#include <vector>
#include <fstream>
#include <utility>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "Scheduler.h"
#include "timestamp.h"
#include "mypaths.h"
#include "Terminal.h"
#include "random.h"
#include "log.h"
#include "Standard.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class param_vec                                                            //
//                                                                            //
// vector of parameters                                                       //
// This is a virtual class, which is a base for template param_vec            //
////////////////////////////////////////////////////////////////////////////////
class param_vec {
protected:
  string name; 
  string unit; 

  virtual ostream& put(ostream& os) const = 0;

public:

  virtual ~param_vec() {};

  virtual bool next() = 0;           // one iteration over parameter vector
  virtual void reset() = 0;          // reset iterator
  virtual unsigned size() const = 0; // return number of elements

  string get_name () {return name;}
  virtual string get_val (unsigned field_width, unsigned double_prec) const = 0;
  string get_unit () {return unit;}

  friend ostream& operator<< (ostream& os, const param_vec& pv) {
    return pv.put(os);
  }
};

////////////////////////////////////////////////////////////////////////////////
// template <class T> class param_vec_                                        //
//                                                                            //
// implementation for different classes T of virtual class param_vec          //
////////////////////////////////////////////////////////////////////////////////
template <class T> class param_vec_ : public param_vec {
  vector<T> vec;
  typename vector<T>::iterator it;

public:

  virtual ~param_vec_() {};

  param_vec_() {it = vec.begin();};

  void init(const string& s, T val, const string& u = "");
  // initialize parameter vector with name 's', initial value 'val' and
  // optional unit 'u'. Initial vector contains just one element.

  bool next(); // one iteration over parameter vector

  bool read_vec (string str);
  // reads parameter vector from string 'str', returns false if an input error
  // occur, true otherwise

  template <class Pred> bool read_vec (string str, Pred p);
  // reads parameter vector from string 'str', returns false if an input error
  // occur or if predicate 'p' is true for any element, true otherwise

  ostream& put(ostream& os) const;
  // output current vector element (includes name and unit)

  string get_val (unsigned field_width, unsigned double_prec) const;
  // returns current element (value only), with given field width and precision
  // in case of floating-point numbers
  
  void reset() {it = vec.begin();} // reset iterator

  T current() const {return *it;} // return current value

  vector<T>& get_vec() {return vec;}

  unsigned size() const {return vec.size();} // return number of elements
};

////////////////////////////////////////////////////////////////////////////////
// parameter vectors for different types                                      //
////////////////////////////////////////////////////////////////////////////////
typedef param_vec_<arrival_time_type> param_vec_arrival_time;
typedef param_vec_<bool> param_vec_bool;
typedef param_vec_<double> param_vec_double;
typedef param_vec_<int> param_vec_int;
typedef param_vec_<transmission_mode> param_vec_transmission_mode;
typedef param_vec_<adapt_mode> param_vec_adapt_mode;
typedef param_vec_<unsigned> param_vec_unsigned;
typedef param_vec_<unsigned long> param_vec_unsigned_long;
typedef param_vec_<discrete_prob> param_vec_discrete_prob;
typedef param_vec_<log_type> param_vec_log_type;
typedef param_vec_<Position> param_vec_Position;
typedef param_vec_<dot11_standard> param_vec_dot11_standard;
typedef param_vec_<channel_bandwidth> param_vec_bandwidth;
typedef param_vec_<channel_model> param_vec_model;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Parameters                                                           //
//                                                                            //
// stores simulation parameters                                               //
//                                                                            //
// Usage:                                                                     //
//  - Default parameters are considered upon instanciation.                   //
//    Parameters can be loaded from file using function 'read_param'.         //
//  - For each parameter different values can be given.                       //
//    By calling 'new_iteration', this class will iterate over all possible   //
//    parameter combinations. The values for each iteration can be obtained   //
//    by calling the 'get_ functions for the desired parameter.               //
//    'reset_iterations' will return iterator to the initial values.          //
//  - new simulation parameters may be included by:                           //
//    1 - declaring a param_vec_ object in the Parameters class.              //
//    2 - declaring and defining the corresponding 'get_' function.           //
//    3 - modifying Parameters.cpp to include parameter in functions          //
//        'assign' and 'default_config'                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
class Parameters {

  vector<param_vec*> it_priority;
  vector<string> iter_str;

  ////////////////////////////////
  // simulation control parameters
  param_vec_unsigned_long Seed; // seeds of random number generator
  timestamp MaxSimTime; // maximum simulation time
  timestamp TempOutputInterval; // interval for temporary outputs and display
  param_vec_log_type  Log;
  param_vec_bool partResults;
  double Confidence; // for calculation of confidence interval
  timestamp TransientTime; // transient time to be ignored
  
  ////////////////////////////////
  // standard
  param_vec_dot11_standard standard;

  ///////////////////////
  // system configuration
  param_vec_unsigned NumberAPs; // number of access points
  param_vec_unsigned NumberStas; // number of mobile stations
  param_vec_double Radius; // cell radius in meters
  vector<param_vec_Position> APPosition; // position of APs
  
  ///////////////////
  // PHY parameters
  param_vec_double TxPowerMax_dBm;
  param_vec_double TxPowerMin_dBm;
  param_vec_double TxPowerStepUp_dBm;
  param_vec_double TxPowerStepDown_dBm;
  param_vec_double CCASensitivity_dBm;
  param_vec_double TargetPER;
  param_vec_bandwidth Bandwidth;
  param_vec_bool shortGI;

  ///////////////////////////////
  // Link Adaptation parameters
  param_vec_transmission_mode TxMode;
  param_vec_adapt_mode AdaptMode;
  param_vec_unsigned LAMaxSucceedCounter;
  param_vec_unsigned LAFailLimit;
  param_vec_bool UseRxMode;

  /////////////////////
  // traffic parameters
  param_vec_discrete_prob PacketLength; // number of data bytes per packet
  param_vec_double DataRate; // in bps
  param_vec_arrival_time ArrivalTime;
  param_vec_double DownlinkFactor;
  param_vec_double UplinkFactor;

  /////////////////////
  // channel parameters
  param_vec_double LossExponent;
  param_vec_double RefLoss_dB;
  param_vec_double NoiseDensity_dBm;
  param_vec_double DopplerSpread_Hz;
  param_vec_unsigned NumberSinus;
  param_vec_model	ChannelModel;

  /////////////////////////
  // IEEE 802.11 MAC parameters
  param_vec_unsigned RTSThreshold;
  param_vec_unsigned RetryLimit;
  param_vec_unsigned FragmentationThresh;
  param_vec_unsigned QueueSize;

  // IEEE 802.11n EDCA parameters
  param_vec_double ppAC_BK;
  param_vec_double ppAC_BE;
  param_vec_double ppAC_VI;
  param_vec_double ppAC_VO;
  param_vec_double ppLegacy;

  // IEEE 802.11n Block ACK and aggregation
  param_vec_bool set_BA_agg;

  bool default_flag; // true if default parameters are stored

  void default_config(); // assigns default configuration values
  
  bool assign (string s1,string s2);
  // assigns value s2 to a configuration field s1.
  // returns false if field does not exist or if value is not valid.

  bool is_consistent ();
  // checks consistency of parameters
  
public:
  Parameters();

  ////////////////////////////////
  // return configuration fields
  adapt_mode get_AdaptMode() {return AdaptMode.current();}  
  arrival_time_type get_ArrivalTime() {return ArrivalTime.current();}
  double get_CCASensitivity() {return CCASensitivity_dBm.current();}
  double get_Confidence() {return Confidence;}
  double get_DataRateDL() {return DataRate.current() * DownlinkFactor.current()
                                                     * 1.0e6;}
  double get_DataRateUL() {return DataRate.current() * UplinkFactor.current()
                                                     * 1.0e6;}
  double get_DopplerSpread() {return DopplerSpread_Hz.current();}
  unsigned get_FragmentationThresh() {return FragmentationThresh.current();}
  unsigned get_LAMaxSucceedCounter() {return LAMaxSucceedCounter.current();}
  unsigned get_LAFailLimit() {return LAFailLimit.current();}
  vector<log_type>& get_Log() {return Log.get_vec();}
  double get_LossExponent() {return LossExponent.current();}
  timestamp get_MaxSimTime() {return MaxSimTime;}
  double get_NoiseDensity() {return NoiseDensity_dBm.current();}
  unsigned get_NumberAPs() {return NumberAPs.current();}
  unsigned get_NumberSinus() {return NumberSinus.current();}
  unsigned get_NumberStas() {return NumberStas.current();}
  discrete_prob get_PacketLength() {return PacketLength.current();}
  unsigned get_QueueSize() {return QueueSize.current();}
  double get_Radius() {return Radius.current();}
  double get_RefLoss() {return RefLoss_dB.current();}
  unsigned get_RetryLimit() {return RetryLimit.current();}
  unsigned get_RTSThreshold() {return RTSThreshold.current();}
  unsigned long get_Seed() {return Seed.current();}
  double get_TargetPER() {return TargetPER.current();}
  timestamp get_TempOutputInterval() {return TempOutputInterval;}
  timestamp get_TransientTime() {return TransientTime;}
  transmission_mode get_TxMode() {return TxMode.current();}
  double get_TxPowerMax() {return TxPowerMax_dBm.current();}
  double get_TxPowerMin() {return TxPowerMin_dBm.current();}
  double get_TxPowerStepDown() {return TxPowerStepDown_dBm.current();}
  double get_TxPowerStepUp() {return TxPowerStepUp_dBm.current();}
  bool get_UseRxMode() {return UseRxMode.current();}
  double get_ppAC_BK() {return ppAC_BK.current();}
  double get_ppAC_BE() {return ppAC_BE.current();}
  double get_ppAC_VI() {return ppAC_VI.current();}
  double get_ppAC_VO() {return ppAC_VO.current();}
  double get_ppLegacy() {return ppLegacy.current();}
  bool get_set_BA_agg() {return set_BA_agg.current();}
  bool get_partResults() {return partResults.current();}
  dot11_standard get_standard() {return standard.current();}
  channel_bandwidth get_bandwidth() {return Bandwidth.current();}
  bool get_shortGI() {return shortGI.current();}
  channel_model get_channelModel() {return ChannelModel.current();}

  Position get_APPosition(unsigned which_ap);
  // returns position of AP number 'which_ap'
  
  unsigned get_number_of_Seeds()     {return Seed.size();}
  // returns number of different seeds

  bool is_default () {return default_flag;}
  // returns true if default configuration is employed

  bool new_iteration();
  // iterates once over simulation parameters, returns false if class has 
  // already iterated over all possible combinations

  bool read_param (const string dir);
  // read parameters from file 'config.txt' in directory 'dir'
  // returns false if it was not possible

  void reset_iterations();
  // go back to first iteration

  /////////////////////////////////////////
  // outputs current simulation parameters 
  friend ostream& operator<< (ostream& os, const Parameters& p);
  vector<string> get_iter_pnames() const;
  vector<string> get_iter_punits() const;
  vector<string> get_param_str(unsigned field_width, unsigned double_prec);
};

#endif


