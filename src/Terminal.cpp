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
#include <sstream>
#include <iomanip>

#include "Terminal.h"
#include "Packet.h"
#include "Profiler.h"
#include "timestamp.h"
#include "math.h"
#include "myexception.h"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// abstract class Terminal                                                    //
////////////////////////////////////////////////////////////////////////////////

unsigned Terminal_private::nterm = 0;

////////////////////////////////////////////////////////////////////////////////
// Terminal constructor                                                       //
////////////////////////////////////////////////////////////////////////////////
Terminal::Terminal(Position p, Scheduler* s, Channel* c, random* r, log_file* l,
                   mac_struct mac, PHY_struct phy, timestamp transient) {

  where = p;
  
  ptr2sch = s;
  randgen = r;
  mylog = l;
  
  transient_time = transient;
  
  id = nterm++;

  // Map MACs and PHYs to ACs
  for(int k = 0; k < 5 ; k++){
	  accCat auxAC = allACs[k];
	  myMACmap[auxAC] = new MAC(this, s, r, l, mac, auxAC);
	  myPHYmap[auxAC] = new PHY(this, p, c, r, s, l, phy);
	  mymac = myMACmap[auxAC];
	  myphy = myPHYmap[auxAC];
	  mymac->connect(myphy);
	  myphy->connect(mymac);
  }

  n_tx_bytes = 0;
  n_tx_packets = 0;
  transfer_delay = transmission_delay = timestamp(0);
  transmission_delay_E2 = transfer_delay_E2 = 0;
  n_pck_lost_q = n_pck_lost_r = 0;
  n_att_packets = 0;
  queue_length = 0;

}

Terminal::~Terminal() {
	for(int k = 0; k < 5 ; k++){
		accCat auxAC = allACs[k];
		map<accCat, PHY*>::const_iterator it = myPHYmap.find(auxAC);
		delete it->second;
	}
	delete myphy;
	delete mymac;
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_average_power                                                //
//                                                                            //
// returns average transmission power in dBm                                  //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_average_power() const {

	double energy = 0;

	for(int k = 0; k < 5 ; k++){
		accCat auxAC = allACs[k];
		map<accCat, PHY*>::const_iterator it = myPHYmap.find(auxAC);
		energy = energy + (it->second)->get_energy();
	}

	return energy / double(ptr2sch->now());
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_n_bytes                                                      //
//                                                                            //
// returns number of bytes successfully sent                                  //
////////////////////////////////////////////////////////////////////////////////
long_integer Terminal::get_n_bytes() const {
  return n_tx_bytes;
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_n_packets                                                    //
//                                                                            //
// returns number of packets successfully sent                                //
////////////////////////////////////////////////////////////////////////////////
unsigned long Terminal::get_n_packets() const {
  return n_tx_packets;
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_n_packets_att                                                //
//                                                                            //
// returns number of attempted MPDU (fragments) transmissions                 //
////////////////////////////////////////////////////////////////////////////////
unsigned long Terminal::get_n_packets_att() const {
	unsigned long att = 0;

	for(int k = 0; k < 5 ; k++){
		accCat auxAC = allACs[k];
		map<accCat, MAC*>::const_iterator it = myMACmap.find(auxAC);
		att = att + (it->second)->get_n_packets_att();
	}

	return att;
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_packet_loss_rate                                             //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_packet_loss_rate() const {
  if (n_pck_lost_r + n_tx_packets == 0) return HUGE_VAL;
  return (double(n_pck_lost_r) / (n_pck_lost_r + n_tx_packets));
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_overflow_rate                                                //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_overflow_rate() const {
  if (n_pck_lost_q + n_tx_packets == 0) return HUGE_VAL;
  return (double(n_pck_lost_q) / (n_pck_lost_q + n_tx_packets));
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_queue_length                                                 //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_queue_length() const {
  if (n_att_packets == 0) return HUGE_VAL;
  return (double(queue_length)/double(n_att_packets));
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_transfer_delay                                               //
//                                                                            //
// returns average transfer delay in sec                                      //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_transfer_delay() const {
  if (n_tx_packets == 0) return HUGE_VAL;
  else return (double(transfer_delay)/double(n_tx_packets));
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_transfer_delay_std                                           //
//                                                                            //
// returns standard deviation of transfer delay                               //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_transfer_delay_std() const{
  if (n_tx_packets == 0) return HUGE_VAL;
  else return sqrt(transfer_delay_E2 / n_tx_packets
               - get_transfer_delay() * get_transfer_delay());
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_transmission_delay                                           //
//                                                                            //
// returns average transmission delay in sec                                  //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_transmission_delay() const{
  if (n_tx_packets == 0) return HUGE_VAL;
  else return (double(transmission_delay)/double(n_tx_packets));
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_transmission_delay_std                                       //
//                                                                            //
// returns standard deviation of transmission delay                           //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_transmission_delay_std() const{
  if (n_tx_packets == 0) return HUGE_VAL;
  else return sqrt(transmission_delay_E2 / n_tx_packets
               - get_transmission_delay() * get_transmission_delay());
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::get_tx_data_rate                                                 //
//                                                                            //
// returns average transmission rate of PHY in Mbps                           //
////////////////////////////////////////////////////////////////////////////////
double Terminal::get_tx_data_rate() const {

	double auxTxDR = 0;

	for(int k = 0; k < 5 ; k++){
		accCat auxAC = allACs[k];
		map<accCat, MAC*>::const_iterator it = myMACmap.find(auxAC);
		auxTxDR = auxTxDR + (it->second)->get_tx_data_rate();
	}

	return auxTxDR;
}

////////////////////////////////////////////////////////////////////////////////
// Terminal::macUnitdataStatusInd                                             //
//                                                                            //
// ACK received, packet 'p' was transmitted correctly.                        //
// 'ack_delay' is the time interval between packet reception at target and    //
// ACK reception at source.                                                   //
////////////////////////////////////////////////////////////////////////////////
void Terminal::macUnitdataStatusInd(MSDU p, timestamp ack_delay) {

  if (ptr2sch->now() < transient_time) return;
  
  //////////////////////////////
  // update performance results
  n_tx_bytes += p.get_nbytes();
  n_tx_packets++;


  timestamp tr_delay_aux = ptr2sch->now() - p.get_time_created() - ack_delay;
  transfer_delay += tr_delay_aux;
  transfer_delay_E2 += double(tr_delay_aux) * double(tr_delay_aux);
  
  timestamp tx_delay_aux = ptr2sch->now() - p.get_tx_time() - ack_delay;
  transmission_delay += tx_delay_aux;
  transmission_delay_E2 += double(tx_delay_aux) * double(tx_delay_aux);

}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// friend functions of class Terminal                                         //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// connect_two                                                                //
//                                                                            //
// establishes connection between two terminals through wireless channel      //
////////////////////////////////////////////////////////////////////////////////
void connect_two (Terminal* t1,  accCat AC1, Terminal* t2, accCat AC2, Channel* ch, adapt_struct ad,
                  traffic_struct tr1to2, traffic_struct tr2to1) {

  t1->connect(t2, ad, tr1to2, AC1);
  t2->connect(t1, ad, tr2to1, AC2);

  // add a new link to the channel
  ch->new_link(t1->myphy,t2->myphy);
}

////////////////////////////////////////////////////////////////////////////////
// output operator<<                                                          //
////////////////////////////////////////////////////////////////////////////////
ostream& operator<< (ostream& os, const Terminal& t) {
  return os << t.str();
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class MobileStation                                                        //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// MobileStation constructor                                                     //
////////////////////////////////////////////////////////////////////////////////
MobileStation::MobileStation(Position p, Scheduler* s, Channel* c, random* r, log_file* l,
		mac_struct mac, accCat AC, PHY_struct phy, timestamp tr)
		: Terminal(p, s, c, r, l, mac, phy, tr) {

	connected = 0;
	myAC = AC;

	mymac = myMACmap[myAC];
	myphy = myPHYmap[myAC];

};
////////////////////////////////////////////////////////////////////////////////
// MobileStation destructor                                                   //
////////////////////////////////////////////////////////////////////////////////
MobileStation::~MobileStation () {
  delete tr;
}

////////////////////////////////////////////////////////////////////////////////
// MobileStation::connect                                                     //
//                                                                            //
// creates connection to another terminal                                     //
////////////////////////////////////////////////////////////////////////////////
void MobileStation::connect(Terminal* t, adapt_struct ad, traffic_struct ts, accCat AC) {
	if (connected) {
		throw(my_exception("second connection attempted to a Mobile Station"));
	}

	connected = t;

	tr = new Traffic(ptr2sch, randgen, mylog, this, t, ts);
	la = link_adapt(this, t, ad, mylog);

}

////////////////////////////////////////////////////////////////////////////////
// void MobileStation::macUnitdataReq(MSDU p);                                     //
//                                                                            //
// new packet was sent to MAC queue, update queue size                        //
////////////////////////////////////////////////////////////////////////////////
void MobileStation::macUnitdataReq(MSDU p) {

	unsigned long l = mymac->macUnitdataReq(p);

	if (ptr2sch->now() < transient_time) return;

	++n_att_packets;
	queue_length += l;
}

////////////////////////////////////////////////////////////////////////////////
// MobileStation::get_connections                                             //
//                                                                            //
// returns string with terminals connected to this one                        //
////////////////////////////////////////////////////////////////////////////////
string MobileStation::get_connections() const {
  return connected->str();
}

////////////////////////////////////////////////////////////////////////////////
// MobileStation::str                                                         //
//                                                                            //
// returns string with terminal type and identification                       //
////////////////////////////////////////////////////////////////////////////////
string MobileStation::str() const {
  ostringstream s;
  s << "Mobile Station " << id;
  return s.str();
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class AccessPoint                                                          //
////////////////////////////////////////////////////////////////////////////////

AccessPoint::AccessPoint(Position p, Scheduler* s, Channel* c, random* r, log_file* l,
		mac_struct mac, PHY_struct phy, timestamp tr)
		: Terminal(p, s, c, r, l, mac, phy, tr) {

	mymac = myMACmap[AC_BK];
	myphy = myPHYmap[AC_BK];

};
////////////////////////////////////////////////////////////////////////////////
// AccessPoint destructor                                                     //
////////////////////////////////////////////////////////////////////////////////
AccessPoint::~AccessPoint() {
  for (
  map<Terminal*, tuple<link_adapt, Traffic*, accCat> >::iterator it = connection.begin();
  it != connection.end(); ++it) {
    delete get<1>(it->second);
  }
}

////////////////////////////////////////////////////////////////////////////////
// AccessPoint::connect                                                       //
//                                                                            //
// creates connection to another terminal                                     //
////////////////////////////////////////////////////////////////////////////////
void AccessPoint::connect(Terminal* t, adapt_struct ad, traffic_struct ts, accCat AC) {

	Traffic* tr = new Traffic(ptr2sch, randgen, mylog, this, t, ts);
	connection[t] = make_tuple(link_adapt(this,t,ad, mylog), tr, AC);

	mymac = myMACmap[AC];
	myphy = myPHYmap[AC];
}

////////////////////////////////////////////////////////////////////////////////
// void AccessPoint::macUnitdataReq(MSDU p);                                     //
//                                                                            //
// new packet was sent to MAC queue, update queue size                        //
////////////////////////////////////////////////////////////////////////////////
void AccessPoint::macUnitdataReq(MSDU p) {

	Terminal* to = p.get_target();
	accCat auxAC = get<2>(connection[to]);

	mymac = myMACmap[auxAC];
	myphy = myPHYmap[auxAC];

	unsigned long l = mymac->macUnitdataReq(p);

	if (ptr2sch->now() < transient_time) return;

	++n_att_packets;
	queue_length += l;
}

////////////////////////////////////////////////////////////////////////////////
// AccessPoint::get_connections                                               //
//                                                                            //
// returns string with terminals connected to this one                        //
////////////////////////////////////////////////////////////////////////////////
string AccessPoint::get_connections() const {

  map<Terminal*, tuple<link_adapt, Traffic*,accCat> >::const_iterator it =
                                                             connection.begin();
  string s = (it++->first)->str();

  map<Terminal*, tuple<link_adapt, Traffic*,accCat> >::const_iterator it_aux =
                                                               connection.end();
  it_aux--;
                                                                 
  for (; it != connection.end(); ++it) {
    if (it == it_aux) s += " and ";
    else s += ", ";

    s += (it->first)->str();
  }

  return s;
}

////////////////////////////////////////////////////////////////////////////////
// AccessPoint::get_current_mode                                              //
////////////////////////////////////////////////////////////////////////////////
transmission_mode AccessPoint::get_current_mode(Terminal* t,unsigned pl) {
  map<Terminal*, tuple<link_adapt, Traffic*, accCat> >::iterator it = connection.find(t);
  if (it == connection.end())
    throw(my_exception(GENERAL,
                       "unknown terminal in AccessPoint::get_current_mode"));
                       
  return (get<0>(it->second)).get_current_mode(pl);
}

////////////////////////////////////////////////////////////////////////////////
// AccessPoint::get_power                                                     //
////////////////////////////////////////////////////////////////////////////////
double AccessPoint::get_power(Terminal* t, unsigned pl) {
  map<Terminal*, tuple<link_adapt, Traffic*, accCat> >::iterator it = connection.find(t);
  if (it == connection.end())
    throw(my_exception(GENERAL,
                       "unknown terminal in AccessPoint::get_current_mode"));
                       
  return (get<0>(it->second)).get_power(pl);

}

////////////////////////////////////////////////////////////////////////////////
// AccessPoint::str                                                           //
//                                                                            //
// returns string with terminal type and identification                       //
////////////////////////////////////////////////////////////////////////////////
string AccessPoint::str() const {
  ostringstream s;
  s << "Access Point " << id;
  return s.str();
}

