#ifndef _MAC_private_h
#define _MAC_private_h 1

#include "Scheduler.h"
#include "Packet.h"
#include "random.h"
#include "log.h"
#include <queue>

class PHY;
class Terminal;

////////////////////////////////////////////////////////////////////////////////
// class MAC_private                                                          //
//                                                                            //
// declares private member objects and functions of class MAC                 //
////////////////////////////////////////////////////////////////////////////////
class MAC_private {
protected:
  Scheduler* ptr2sch;  // pointer to simulation scheduler
  random*    randgen;  // pointer to random number generator

  queue<MSDU> packet_queue;
  
  log_file*  mylog;
  bool       logflag;  // true if MAC events should be logged
  
  PHY* myphy;      // pointer to physical layer
  Terminal* term;  // pointer to owner terminal

  transmission_mode mode;  // transmission rate of current fragment train

  double power_dBm;          // power in dBm for current train of MPDUs
  MPDU pck;                  // currently transmitted packet (MPDU)
  MSDU msdu;                 // currently transmitted packet (MSDU)
  transmission_mode rx_mode; // transmission rate of latest received packet

  // MAC variables
  unsigned  backoff_counter;
  unsigned  contention_window;
  timestamp time_to_send;      // time scheduled for next transmission
  bool      countdown_flag;    // true if backoff countdown is activated
  unsigned  retry_count;       // number of transmissions retry for current MSDU
  timestamp NAV;               // network allocation vector
  timestamp NAV_RTS;           // duration of packet requested to send
  unsigned  nfrags;            // number of fragments in current packet
  unsigned  current_frag;      // latest transmitted fragment

  // MAC parameters
  unsigned  retry_limit;       
  unsigned  RTS_threshold;
  unsigned  frag_thresh;
  unsigned  max_queue_size;

  // performance measures
  unsigned long n_att_frags;  // number of data fragments transmission attempts
  double        tx_data_rate; // mean transmitted data rate

  void ack_timed_out();
  
  void begin_countdown();

  void check_nav();
  // check if channel is free despite NAV (when RTS was detected)

  void cts_timed_out();

  void end_nav();
  // channel released according to NAV

  void new_msdu();
  // transmits next packet in queue
  
  void receive_bc(MPDU p); 
  // receive message targeted at other terminal, update NAV if needed
  
  void receive_this(MPDU p);
  // receive message targeted at this terminal

  void send_ack(Terminal *to);
  void send_cts(Terminal *to);
  void send_data();
  
  void transmit();
  // send data packet or begin RTS
  
  void tx_attempt();
  // begin contention for new MSDU or for new train of fragments 
    
public:
  
  /////////////////////////////////////////////////////
  // wrapper functions for member-function call-back //
  /////////////////////////////////////////////////////
  static void wrapper_to_ack_timed_out (void* ptr2obj) {
    ((MAC_private*)ptr2obj)->ack_timed_out();}

  static void wrapper_to_check_nav (void* ptr2obj) {
    ((MAC_private*)ptr2obj)->check_nav();}

  static void wrapper_to_cts_timed_out (void* ptr2obj) {
    ((MAC_private*)ptr2obj)->cts_timed_out();}

  static void wrapper_to_end_nav (void* ptr2obj) {
    ((MAC_private*)ptr2obj)->end_nav();}

  static void wrapper_to_send_ack (void* ptr2obj, void* param) {
    ((MAC_private*)ptr2obj)->send_ack((Terminal*)param);}

  static void wrapper_to_send_cts (void* ptr2obj, void* param) {
    ((MAC_private*)ptr2obj)->send_cts((Terminal*)param);}

  static void wrapper_to_send_data (void* ptr2obj) {
    ((MAC_private*)ptr2obj)->send_data();}
    
  static void wrapper_to_transmit (void* ptr2obj) {
    ((MAC_private*)ptr2obj)->transmit();}

  static void wrapper_to_tx_attempt (void* ptr2obj) {
    ((MAC_private*)ptr2obj)->tx_attempt();}    
  /////////////////////////////////////////////////////
};

#endif
