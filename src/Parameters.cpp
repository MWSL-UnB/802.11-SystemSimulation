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

#include <fstream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <functional>

#include "timestamp.h"
#include "myexception.h"
#include "Parameters.h"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// template param_vec_                                                        //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// param_vec_::init                                                           //
////////////////////////////////////////////////////////////////////////////////
template <class T> void param_vec_<T>::init(const string& s, T val, const string& u) {
  name = s;
  unit = u;
  vec.assign(1,val);
  it = vec.begin();
}

////////////////////////////////////////////////////////////////////////////////
// param_vec_::next                                                           //
////////////////////////////////////////////////////////////////////////////////
template <class T> bool param_vec_<T>::next() {
  if (++it == vec.end()) {
    it = vec.begin();
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// param_vec_::read_vec                                                       //
////////////////////////////////////////////////////////////////////////////////
template <class T> bool param_vec_<T>::read_vec (string str) {

  string::size_type pos;
  vec.clear();
  do {
    pos = str.find(",");
    string saux(str,0,pos);
    str.erase(0,pos+1);
    istringstream strstr(saux);
    T element;
    if ((strstr >> element).fail()) return false;
    vec.push_back(element);
  } while (pos != string::npos);
  it = vec.begin();

  return true;
}

template <class T>
template <class Pred> bool param_vec_<T>::read_vec (string str, Pred p) {
  string::size_type pos;
  vec.clear();
  do {
    pos = str.find(",");
    string saux(str,0,pos);
    str.erase(0,pos+1);
    istringstream strstr(saux);
    T element;
    if ((strstr >> element).fail() || p(element)) return false;
    vec.push_back(element);
  } while (pos != string::npos);
  it = vec.begin();

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// param_vec_::put                                                            //
////////////////////////////////////////////////////////////////////////////////
template <class T> ostream& param_vec_<T>::put(ostream& os) const {
    return os << name << " = " << *it << " " << unit;
}

////////////////////////////////////////////////////////////////////////////////
// param_vec_::get_val                                                        //
////////////////////////////////////////////////////////////////////////////////
template <class T> 
string param_vec_<T>::get_val (unsigned field_width,
                               unsigned double_prec) const {
  ostringstream str;
  str.precision(double_prec);
  str << setw(field_width) << *it;
  return str.str();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class invalid_prob                                                         //
////////////////////////////////////////////////////////////////////////////////
class invalid_prob : public unary_function<double,bool> {
public:
  explicit invalid_prob() {}
  bool operator() (double p) const {return (p < 0 || p > 1.0);}
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Parameters                                                           //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// constructor                                                                //
////////////////////////////////////////////////////////////////////////////////
Parameters::Parameters() {
  default_config();
}

////////////////////////////////////////////////////////////////////////////////
// Parameters::assign                                                         //
//                                                                            //
// assigns value s2 to a configuration field s1.                              //
// returns false if field does not exist or if value is not valid.            //
////////////////////////////////////////////////////////////////////////////////
bool Parameters::assign (string s1,string s2) {

  param_vec* which_param = 0;

  if (!s1.compare("AdaptMode")){
    which_param = &AdaptMode;
    if (!AdaptMode.read_vec(s2)) return false;

  } else if (!s1.compare("ArrivalTime")){
    which_param = &ArrivalTime;
    if (!ArrivalTime.read_vec(s2)) return false;

  } else if (!string(s1,0,11).compare("APPosition_")){
    unsigned index = atol(string(s1,11,s1.size()-11).c_str());
    if (index < 0 || index >= APPosition.size()) return false;
    which_param = &APPosition[index];
    if (!APPosition[index].read_vec(s2)) return false;

  } else if (!s1.compare("CCASensitivity_dBm")){
    which_param = &CCASensitivity_dBm;
    if (!CCASensitivity_dBm.read_vec(s2)) return false;

  } else if (!s1.compare("Confidence")) {
    if ((Confidence = atof(s2.c_str())) >= 1 || Confidence <= 0) return false;

  } else if (!s1.compare("DataRate")){
    which_param = &DataRate;
    if (!DataRate.read_vec(s2, bind2nd(less<double>(),0))) return false;

  } else if (!s1.compare("DopplerSpread_Hz")){
    which_param = &DopplerSpread_Hz;
    if (!DopplerSpread_Hz.read_vec(s2,bind2nd(less<double>(),0))) return false;

  } else if (!s1.compare("DownlinkFactor")){
    which_param = &DownlinkFactor;
    if (!DownlinkFactor.read_vec(s2,bind2nd(less<double>(),0))) return false;

  } else if (!s1.compare("FragmentationThreshold")){
    which_param = &FragmentationThresh;
    if (!FragmentationThresh.read_vec(s2,bind2nd(less_equal<unsigned>(),0)))
      return false;

  } else if (!s1.compare("LAFailLimit")){
    which_param = &LAFailLimit;
    if (!LAFailLimit.read_vec(s2,bind2nd(less_equal<unsigned>(),0)))
      return false;

  } else if (!s1.compare("LAMaxSucceedCounter")){
    which_param = &LAMaxSucceedCounter;
    if (!LAMaxSucceedCounter.read_vec(s2)) return false;

  } else if (!s1.compare("Log")){
    if (!Log.read_vec(s2)) return false;

  } else if (!s1.compare("LossExponent")){
    which_param = &LossExponent;
    if (!LossExponent.read_vec(s2,bind2nd(less_equal<double>(),0)))
      return false;

  } else if (!s1.compare("MaxSimTime")) {
    MaxSimTime = timestamp(atof(s2.c_str()));

  } else if (!s1.compare("NoiseVariance_dBm")){
    which_param = &NoiseVariance_dBm;
    if (!NoiseVariance_dBm.read_vec(s2)) return false;

  } else if (!s1.compare("NumberAPs")){
    which_param = &NumberAPs;
    if (!NumberAPs.read_vec(s2)) return false;
    
    unsigned maxNumberAPs = 0;
    do {
      if (NumberAPs.current()>maxNumberAPs) maxNumberAPs = NumberAPs.current();
    } while (NumberAPs.next());

    APPosition.clear();
    for (unsigned count = 0; count < maxNumberAPs; ++count) {
      APPosition.push_back(param_vec_Position());
      ostringstream auxs;
      auxs << "Position of AP " << count;
      
      (APPosition.back()).init("auxs",Position(),"m");
    }

  } else if (!s1.compare("NumberSinus")){
    which_param = &NumberSinus;
    if (!NumberSinus.read_vec(s2,bind2nd(less<unsigned>(),1))) return false;

  } else if (!s1.compare("NumberStas")){
    which_param = &NumberStas;
    if (!NumberStas.read_vec(s2,bind2nd(less<unsigned>(),1))) return false;

  } else if (!s1.compare("PacketLength")){
    which_param = &PacketLength;
    if (!PacketLength.read_vec(s2)) return false;

  } else if (!s1.compare("QueueSize")){
    which_param = &QueueSize;
    if (!QueueSize.read_vec(s2)) return false;

  } else if (!s1.compare("Radius")){
    which_param = &Radius;
    if (!Radius.read_vec(s2,bind2nd(less_equal<double>(),0))) return false;

  } else if (!s1.compare("RefLoss_dB")){
    which_param = &RefLoss_dB;
    if (!RefLoss_dB.read_vec(s2)) return false;

  } else if (!s1.compare("RetryLimit")){
    which_param = &RetryLimit;
    if (!RetryLimit.read_vec(s2,bind2nd(less_equal<unsigned>(),0)))
      return false;

  } else if (!s1.compare("RTSThreshold")){
    which_param = &RTSThreshold;
    if (!RTSThreshold.read_vec(s2)) return false;

  } else if (!s1.compare("Seed")){
    if (!Seed.read_vec(s2)) return false;

  } else if (!s1.compare("TargetPER")){
    which_param = &TargetPER;
    if (!TargetPER.read_vec(s2,invalid_prob())) return false;

  } else if (!s1.compare("TempOutputInterval")){
    TempOutputInterval = timestamp(atof(s2.c_str()));

  } else if (!s1.compare("TransientTime")) {
    TransientTime = timestamp(atof(s2.c_str()));

  } else if (!s1.compare("TxMode")){
    which_param = &TxMode;
    if (!TxMode.read_vec(s2)) return false;

  } else if (!s1.compare("TxPowerMax_dBm")){
    which_param = &TxPowerMax_dBm;
    if (!TxPowerMax_dBm.read_vec(s2)) return false;

  } else if (!s1.compare("TxPowerMin_dBm")){
    which_param = &TxPowerMin_dBm;
    if (!TxPowerMin_dBm.read_vec(s2)) return false;

  } else if (!s1.compare("TxPowerStepDown_dBm")){
    which_param = &TxPowerStepDown_dBm;
    if (!TxPowerStepDown_dBm.read_vec(s2,bind2nd(less_equal<double>(),0)))
      return false;

  } else if (!s1.compare("TxPowerStepUp_dBm")){
    which_param = &TxPowerStepUp_dBm;
    if (!TxPowerStepUp_dBm.read_vec(s2,bind2nd(less_equal<double>(),0)))
      return false;

  } else if (!s1.compare("UplinkFactor")){
    which_param = &UplinkFactor;
    if (!UplinkFactor.read_vec(s2,bind2nd(less<double>(),0))) return false;

  } else if (!s1.compare("UseRxMode")){
    which_param = &UseRxMode;
    if (!UseRxMode.read_vec(s2)) return false;

  } else if (!s1.compare("ppAC_BK")) {
	  which_param = &ppAC_BK;
	  if (!ppAC_BK.read_vec(s2)) return false;

  } else if (!s1.compare("ppAC_BE")) {
  	  which_param = &ppAC_BE;
  	  if (!ppAC_BE.read_vec(s2)) return false;

  } else if (!s1.compare("ppAC_VI")) {
  	  which_param = &ppAC_VI;
  	  if (!ppAC_VI.read_vec(s2)) return false;

  } else if (!s1.compare("ppAC_VO")) {
	  which_param = &ppAC_VO;
	  if (!ppAC_VO.read_vec(s2)) return false;

  } else if (!s1.compare("ppLegacy")) {
	  which_param = &ppLegacy;
	  if (!ppLegacy.read_vec(s2)) return false;

  } else if (!s1.compare("set_BA_agg")) {
	  which_param = &set_BA_agg;
	  if (!set_BA_agg.read_vec(s2)) return false;

  } else if (!s1.compare("partResults")) {
  	  which_param = &partResults;
  	  if (!partResults.read_vec(s2)) return false;

  } else if (!s1.compare("Standard")) {
	  which_param = &standard;
	  if (!standard.read_vec(s2)) return false;

  } else if (!s1.compare("Bandwidth")) {
  	  which_param = &Bandwidth;
  	  if (!Bandwidth.read_vec(s2)) return false;

  } else {
    return false;
  }

  if (which_param && which_param->size() > 1)
    it_priority.push_back(which_param);

  return true;

}

////////////////////////////////////////////////////////////////////////////////
// Parameters::default_config                                                 //
//                                                                            //
// default configuration                                                      //
////////////////////////////////////////////////////////////////////////////////
void Parameters::default_config () {
  default_flag = true;

  TempOutputInterval = not_a_timestamp();
  MaxSimTime = timestamp(0);
  Confidence = .95;
  TransientTime = timestamp(0);
  Seed.init("seed",1);


  APPosition.assign(1,param_vec_Position());
  (APPosition.front()).init( "Position of AP 0",Position(),"m");
  NumberAPs.init("number of APs",1);
  NumberStas.init("number of Stas",10);
  Radius.init("cell radius",10.0,"meters");

  PacketLength.init("packet length",500,"data bytes");
  DataRate.init("data rate",1.0,"Mbps");
  ArrivalTime.init("arrival time distribution",EXP);
  UplinkFactor.init("uplink factor",1.0); // offered load in Mbps
  DownlinkFactor.init("downlink factor",1.0); // offered load per link in Mbps

  // channel parameters
  LossExponent.init("path loss exponent",3.0);
  RefLoss_dB.init("path loss at 1m",47.0,"dB");
  NoiseVariance_dBm.init("noise variance",-95.0,"dBm");
  DopplerSpread_Hz.init("Doppler spread",0,"Hz");
  NumberSinus.init("Number of sinusoidals",20);

  // PHY parameters
  TxPowerMax_dBm.init("maximum transmit power",0,"dBm");
  TxPowerMin_dBm.init("minimum transmit power",0,"dBm");
  TxPowerStepUp_dBm.init("transmit power step (up)",1,"dBm");
  TxPowerStepDown_dBm.init("transmit power step (down)",1,"dBm");
  TxMode.init("transmit mode",MCS0);
  AdaptMode.init("adaptation mode",RATE);
  Bandwidth.init("bandwidth",MHz20);

  TargetPER.init("target PER",.1);

  LAMaxSucceedCounter.init("succeed counter limit",0);
  LAFailLimit.init("fail counter limit",1);
  UseRxMode.init("use reception mode (Y/N)",false);

  CCASensitivity_dBm.init("receiver CCA sensitivity",-82,"dBm");

  // IEEE802.11 MAC-parameters
  RTSThreshold.init("RTS threshold",10);
  RetryLimit.init("retry limit",10);
  FragmentationThresh.init("fragmentation threshold",2312);
  QueueSize.init("queue size",100);

  // IEEE802.11n EDCA parameters
  ppAC_BK.init("proportion of AC_BKs",0);
  ppAC_BE.init("proportion of AC_BEs",0);
  ppAC_VI.init("proportion of AC_VIs",0);
  ppAC_VO.init("proportion of AC_VOs",0);
  ppLegacy.init("proportion of legacies",NumberStas.current());

  set_BA_agg.init("BA and Aggregation Flag",false);

  standard.init("802.11 Standard",dot11a);

}

////////////////////////////////////////////////////////////////////////////////
// Parameters::get_APPosition                                                 //
//                                                                            //
// returns position of AP number 'which_ap'                                   //
////////////////////////////////////////////////////////////////////////////////
Position Parameters::get_APPosition(unsigned which_ap) {

  if (which_ap < 0 || which_ap >= APPosition.size()) {
    throw(my_exception("Invalid AP number in Parameters::get_APPosition"));
  } else {
    return((APPosition[which_ap]).current());
  }
}

////////////////////////////////////////////////////////////////////////////////
// Parameters::is_consistent                                                  //
//                                                                            //
// checks consistency of parameters                                           //
////////////////////////////////////////////////////////////////////////////////
bool Parameters::is_consistent() {
/*	if(ppAC_BK.current()+ppAC_BE.current()+ppAC_VI.current()+ppAC_VO.current()+
			ppLegacy.current() > 1) {
		return false;
	}
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Parameters::new_iteration                                                  //
//                                                                            //
// iterates once over simulation parameters, returns false if class has       //
// already iterated over all possible combinations                            //
////////////////////////////////////////////////////////////////////////////////
bool Parameters::new_iteration() {
  // iteration inner loop over all seeds
  if (Seed.next())
    return true;
  else {
    for (vector<param_vec*>::reverse_iterator rit = it_priority.rbegin();
         rit != it_priority.rend(); rit++) if ((*rit)->next()) return true;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////
// Parameters::read_param                                                     //
//                                                                            //
// read parameters from file 'config.txt' in directory 'dir'                  //
// returns false if it was not possible                                       //
////////////////////////////////////////////////////////////////////////////////
bool Parameters::read_param (const string dir) {
  try {
    default_flag = false;

    string filename = dir + '\\' + CONFIG_FILE_NAME;

    ifstream config_file;
    config_file.open(filename.c_str());
    if (!config_file.is_open()) throw my_exception(OPENFILE,filename);

    string auxstr;

    unsigned line_number = 0;

    // read from file
    while (!config_file.eof()) {
      line_number++;
      getline(config_file,auxstr);

      // Check for comments starting with '%'
      size_t pos;
      if ((pos = auxstr.find("%"))!=auxstr.npos) {
        auxstr.erase(pos);
      }

      // Eliminate all whitespaces
      while ((pos=auxstr.find_first_of(" \t"))!=auxstr.npos) {
        auxstr.erase(pos,1);
      }

      // Check if line is empty
      if (!auxstr.empty()) {
        // Check if one (and only one) '=' is found
        if ((pos=auxstr.find_first_of("="))!=auxstr.npos) {
          if (auxstr.find_last_of("=")!=pos) {
            throw my_exception(SYNTAX,line_number);
          }

          // parse variable name and value
          string str1;
          str1.assign(auxstr,0,pos);
          string str2;
          str2.assign(auxstr,pos+1,auxstr.length());

          // assign value to variable
          if (!assign (str1,str2)) {
            throw my_exception(SYNTAX,line_number);
          }
        } else throw my_exception(SYNTAX,line_number);
      }

    }
  }
  catch (my_exception& e) {
    cout <<  e.what();
    return false;
  }
    
  return is_consistent();
}

////////////////////////////////////////////////////////////////////////////////
// Parameters::reset_iterations                                               //
//                                                                            //
// go back to first iteration                                                 //
////////////////////////////////////////////////////////////////////////////////
void Parameters::reset_iterations() {
  Seed.reset();
  for (vector<param_vec*>::iterator it = it_priority.begin();
       it != it_priority.end(); it++) (*it)->reset();
}

////////////////////////////////////////////////////////////////////////////////
// output operator<<                                                          //
////////////////////////////////////////////////////////////////////////////////
ostream& operator<< (ostream& os, const Parameters& p) {

  for (vector<param_vec*>::const_iterator it = (p.it_priority).begin();
       it != (p.it_priority).end(); it++) os << **it << ", ";
  if ((p.Seed).size() > 1) os << p.Seed;

  return os;
}

////////////////////////////////////////////////////////////////////////////////
// Parameters::get_iter_pnames                                                //
////////////////////////////////////////////////////////////////////////////////
vector<string> Parameters::get_iter_pnames() const {
  vector<string> vec;
  for (vector<param_vec*>::const_iterator it = it_priority.begin();
       it != it_priority.end(); ++it) {
    vec.push_back((*it)->get_name());
  }

  return vec;
}

////////////////////////////////////////////////////////////////////////////////
// Parameters::get_iter_punits                                                //
////////////////////////////////////////////////////////////////////////////////
vector<string> Parameters::get_iter_punits() const {
  vector<string> vec;
  for (vector<param_vec*>::const_iterator it = it_priority.begin();
       it != it_priority.end(); ++it) {
    vec.push_back((*it)->get_unit());
  }

  return vec;
}

////////////////////////////////////////////////////////////////////////////////
// Parameters::get_param_str                                                  //
////////////////////////////////////////////////////////////////////////////////
vector<string> Parameters::get_param_str(unsigned field_width,
                                         unsigned double_prec) {
  vector<string> vec;
  for (vector<param_vec*>::const_iterator it = it_priority.begin();
       it != it_priority.end(); ++it) {
    vec.push_back((*it)->get_val(field_width, double_prec));
  }

  return vec;
}
