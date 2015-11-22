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



