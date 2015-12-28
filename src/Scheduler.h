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

#ifndef _Scheduler_h
#define _Scheduler_h 1

#include <queue>
#include <functional>

#include "timestamp.h"
#include "long_integer.h"


////////////////////////////////////////////////////////////////////////////////
// class Event                                                                //
//                                                                            //
// discrete event                                                             //
//                                                                            //
// Usage:                                                                     //
// - each event is instanced with a time stamp, an action and (optionally) a  //
//   parameter                                                                //
// - action is performed by calling 'go'                                      //
// - instanced events can be deactivated with 'deactivate', in which case     //
//   action will not be performed                                             //
////////////////////////////////////////////////////////////////////////////////
class Event{
      timestamp when; // time at which event should be performed

      void* p_fun; // pointer to function
      void* p_obj; // pointer to object (used with member function)

      // function parameters
      void* p_param;         // pointer parameter
      long_integer li_param; // long_integer parameter
      bool li_param_flag;    // true if a long_integer parameter was defined

      static long_integer event_count; // number of events already created
      long_integer id;                 // event unique identification number

      bool active; // event is only carried out if active==true

public:
  Event();
  Event(timestamp t,  // time at which event should be performed
        void* pf,     // pointer to function to be called back
        void* po = 0, // (optional) pointer to object in case of member function
        void* p1 = 0  // (optional) parameter
        );
  Event(timestamp t,
        void* pf,
        void* po,
        long_integer li // (optional) parameter
        );

  int go(); // perform event

  timestamp    get_time () const {return when;}
  long_integer get_id ()   const {return id;}

  void deactivate() {active = false;}

  bool same_id (long_integer l) const { return(id==l && active);}
  bool same_pointers (void* pf, void* po) const {
    return(pf==p_fun && p_obj==po && active);
  }

  // comparison operators are based on the event timestamp
  bool operator< (Event e) const;
  bool operator<= (Event e) const;
  bool operator> (Event e) const;
  bool operator>= (Event e) const;
};


////////////////////////////////////////////////////////////////////////////////
// class Scheduler                                                            //
//                                                                            //
// discrete-event scheduler                                                   //
//                                                                            //
// Usage:                                                                     //
// - Scheduler consists of a priority queue of Events. Events with lower      //
//   timestamps are called back first. No processing order is guaranteed for  //
//   Events with the same timestamp.                                          //
// - events are added to the scheduler with 'schedule'.                       //
// - events can be removed from scheduler with 'remove'.                      //
// - 'run' starts simulation.                                                 //
////////////////////////////////////////////////////////////////////////////////
class Scheduler
      : private priority_queue< Event,vector<Event>,greater<Event> >{

  timestamp present;
public:
  Scheduler() {}

  void init (); // clear queue

  long_integer schedule(Event e); // add event to scheduler, returns event id

  void remove (long_integer event_id); // removes (deactivates) event
  void remove (void* pf, void* po);

  void run(timestamp tmax); // run scheduler until tmax is reached

  timestamp now() const {return present;} // returns current simulation time
  int n_events () const {return size();}  // returms number of events in queue
};

#endif
