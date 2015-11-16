#ifndef _Simulation_h
#define _Simulation_h 1

#include <string>

#include "Parameters.h"
#include "Scheduler.h"
#include "random.h"
#include "Channel.h"
#include "Terminal.h"
#include "log.h"
#include "DataStatistics.h"

////////////////////////////////////////////////////////////////////////////////
// struct res_struct                                                          //
//                                                                            //
// simulation results                                                         //
////////////////////////////////////////////////////////////////////////////////
struct res_struct {
      
      
 double throughput;
 double transfer_time;
 double transfer_time_std;
 double tx_time;
 double tx_time_std;
 double packet_loss_rate;
 double overflow_rate;
 double queue_length;
 double average_power;


res_struct () : throughput(0), transfer_time(0), transfer_time_std(0) {};


res_struct ( double tp, double tr, double trstd, double tx, double txstd, 
              double plr, double ofr, double ql, double ap):
            throughput(tp), transfer_time(tr), transfer_time_std(trstd),
            tx_time(tx), tx_time_std(txstd), packet_loss_rate(plr),
            overflow_rate(ofr), queue_length(ql), average_power(ap) {};
 

};

////////////////////////////////////////////////////////////////////////////////
// class Simulation                                                           //
//                                                                            //
// controls the simulation iterations and outputs the results                 //
////////////////////////////////////////////////////////////////////////////////
class Simulation {
private:
  Parameters sim_par; 
  Scheduler main_sch; 
  random randgent;
  Channel* ch;
  
  vector<Terminal*> term_vector;

  log_file log;

  string wdir;  // working directory
  ofstream out; // output file
  
  vector<res_struct> results;   // simulation results


  
  void final_results();  // stop simulation and output results
  void init_terminals(); // initialize terminals for a new iteration

  void log_connections(); // output all active communication links

  void run();       // start all simulations
  void start_sim(); // start a new iteration

  void temp_output(); // display results in standard output during simulation
  void wrap_up();     // end one iteration and collect performance results
                      // output them if required (if 'it_file_flag == true')

public:
  Simulation(string dir,      // working directory
             string par = ""  // command-line parameters
            );

  static void wrapper_to_temp_output(void* ptr2obj);
};

#endif
