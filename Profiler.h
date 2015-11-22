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

#ifndef _PROFILER_H
#define _PROFILER_H

#include <windows.h>
#include <map>
#include <vector>
#include <string>
#include <fstream>

#include "myexception.h"

////////////////////////////////////////////////////////////////////////////////
// the comment in the following line must be removed to activate the profiler:
//#define _PROFILE_ _this_profiler_
// and the program must be recompiled.
//
// functions (or pieces of code) to be profiled must have 
//   BEGIN_PROF("function_name")
// at its beginning, and 
//   END_PROF("function_name")
// at each exit point.
//
// Profiling results are output at file "profile.txt"
////////////////////////////////////////////////////////////////////////////////


#ifndef _PROFILE_
#define BEGIN_PROF(Flag)
#define END_PROF(Flag)
#else
#define BEGIN_PROF(Flag) (_PROFILE_).begin(Flag);
#define END_PROF(Flag) (_PROFILE_).end(Flag);

const unsigned str_size = 25;
struct prof_struct{
  __int64 proc_time;
  unsigned long ntimes;

  inline prof_struct() {proc_time = 0;ntimes = 0;}
  inline prof_struct& operator+= (__int64 t) {
    proc_time += t;
    ntimes++;
    return *this;
  }
};

////////////////////////////////////////////////////////////////////////////////
// class Profiler                                                             //
////////////////////////////////////////////////////////////////////////////////
class Profiler {
  map<string, prof_struct> profile;
  vector<__int64> current;
  LARGE_INTEGER t;

  __int64 start;
  double precision; // clocking precision
  __int64 tquery;  // approximate time required for a counter query
  __int64 tcall; // approximate time for a function call
  __int64 tcall2; // approximate time for a function call
  string filename;

public:
  /////////////////
  // Constructor //
  /////////////////
  Profiler(string str = "profile.txt") : filename(str) {

    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    precision = double(f.QuadPart);

    //calibrate
    LARGE_INTEGER t1;
    QueryPerformanceCounter(&t1);
    tquery = 0;
    tcall = 0;
    for (int i = 0; i < 1000; i++) {
      begin("Calibrate");
      end("Calibrate");
    }
    prof_struct aux = profile["Calibrate"];
    tquery = aux.proc_time/aux.ntimes;
    LARGE_INTEGER t2;
    QueryPerformanceCounter(&t2);
    tcall = (t2.QuadPart - t1.QuadPart)/aux.ntimes;


    profile.clear();
    start = t2.QuadPart;
  };

  ////////////////
  // Destructor //
  ////////////////
  ~Profiler() {
    QueryPerformanceCounter(&t);

    __int64 total_time = t.QuadPart - start;

    ofstream out(filename.c_str());
    if (!out.is_open()) throw (OPENFILE, filename);

    map<string,prof_struct>::const_iterator it = profile.begin();
    __int64 real_time_i = total_time;
    while (it != profile.end()) {
      real_time_i -= (it->second).ntimes * tcall;
      it++;
    }
    double real_time = double(real_time_i)/precision;

    out << "Resolution = " << 1.0/precision << " s\n";
    out << "Total processor time = " << double(total_time)/precision
        << " seconds\n";
    out << "Real processor time excluding profiler = " << real_time
        << " seconds\n";
    out << "Estimate time for a query = " << double(tquery)/precision
        << " seconds\n";
    out << "Estimate time for a function call = " << double(tcall)/precision
        << " seconds\n\n\n";

    // sort profile based on processor time
    it = profile.begin();
    multimap<double,pair<string,unsigned long> > mm_aux;
    while (it != profile.end()) {
      mm_aux.insert(make_pair(double((it->second).proc_time)/precision,
                              make_pair(it->first,(it->second).ntimes)));
      it++;
    }

    multimap<double,pair<string,unsigned long> >::reverse_iterator
                                                        it_mm = mm_aux.rbegin();
    while(it_mm != mm_aux.rend()) {
      out << (it_mm->second).first << "\n";
      if (it_mm->first > 0) {
        out << "\tproc. time = " << it_mm->first << " seconds"
            << " (" << it_mm->first / real_time * 100 << "%)"
            << "\n\tcalled " << (it_mm->second).second << " times"
            << "\n\tmean proc. time per call = "
            << it_mm->first / (it_mm->second).second << "seconds\n"
            << endl;
      } else {
        out << "\tproc. time is too small to measure"
            << "\n\tcalled " << (it_mm->second).second << " times";
      }

      it_mm++;
    }

    profile.clear();
    current.clear();
  }

  ///////////
  // begin //
  ///////////
  inline void begin(const string& str) {
    vector<__int64>::iterator it = current.begin();
    while (it != current.end()) {
      *it++ += tcall;
    }

    QueryPerformanceCounter(&t);
    current.push_back(t.QuadPart);
  }

  /////////
  // end //
  /////////
  inline void end(const string& str) {
    QueryPerformanceCounter(&t);

    profile[str] += t.QuadPart - current.back() - tquery;
    current.pop_back();
  }

};

#ifdef _PROFILE_
#ifndef _PROFILER_CPP
extern Profiler _this_profiler_;
#endif
#endif
#endif 
#endif
