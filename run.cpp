#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

#include "Simulation.h"

#include "Scheduler.h"
#include "Terminal.h"
#include "myexception.h"
#include "mypaths.h"
#include "Parameters.h"
#include "Channel.h"

////////////////////////////////////////////////////////////////////////////////
// main function                                                              //
//                                                                            //
// program may be called with the following command-line options:             //
// -no_pause : console does not pause after running                           //
// -it_file  : results for each iteration are saved in files                  //
// dirname   : configuration and results files in directory 'dirname'         //
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
try {
  string wdir = string(W_DIR) + string(DEFAULT_DIR);
  bool pause_flag = true;

  string simstr;

  //////////////////////////////
  // check command-line options
  for (int count = 1; count < argc; count++) {
    string auxstr = argv[count];

    if (auxstr == "-no_pause") {
      pause_flag = false;    
    } else if (auxstr.find("-") == 0) {
      wdir = W_DIR + string(auxstr.begin()+1,auxstr.end());
    } else {
      throw(my_exception(GENERAL,"Invalid command line parameter"));
    }
  }
  
  // start simulation
  Simulation sim(wdir, simstr);

  if (pause_flag) system("pause");
  exit(0);
}
catch (my_exception& e)  {
  cout << e.what() << "\a\a\a\n";
  system ("pause");
  exit(1);
}
catch (out_of_range& e) {
  cout << e.what() << "\a\a\a\n";
  system ("pause");
  exit(1);
}
}



