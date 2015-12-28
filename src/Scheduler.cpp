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

#include <iostream>
#include <algorithm>
#include <functional>

#include "Scheduler.h"
#include "timestamp.h"
#include "long_integer.h"
#include "myexception.h"

typedef void(*ptr2func)();
typedef void(*ptr2func_onepar)(void*);
typedef void(*ptr2func_twopars_li)(void*,long_integer);
typedef void(*ptr2func_twopars)(void*,void*);

////////////////////////////////////////////////////////////////////////////////
// class same_event                                                           //
//                                                                            //
// function-like object to determine if parameters correspond to a given      //
// event                                                                      //
////////////////////////////////////////////////////////////////////////////////
class same_event : public unary_function<Event,bool> {
  long_integer id;
  void* p_fun;
  void* p_obj;
public:
  explicit same_event (long_integer l) : id(l) {}
  explicit same_event (void* pf, void* po) : p_fun(pf), p_obj(po) {
    id = not_a_long_integer;
  }
  bool operator()(const Event& e) const {
    if (id == not_a_long_integer) return e.same_pointers(p_fun, p_obj);
    return e.same_id(id);
  }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Event                                                                //
////////////////////////////////////////////////////////////////////////////////
long_integer Event::event_count = 0;

////////////////////////////////////////////////////////////////////////////////
// Event Constructors                                                         //
////////////////////////////////////////////////////////////////////////////////
Event::Event() {
  p_obj = NULL;
  p_fun = NULL;
  p_param = NULL;

  when = timestamp(0);
  active = false;
}

////////////////////////////////////////////////////////////////////////////////
Event :: Event(timestamp t, void* pf, void* po, void* p1)
              : when(t), p_fun(pf), p_obj(po),
                p_param(p1) {
  id = event_count++;
  li_param_flag = false;
  active = true;
}

////////////////////////////////////////////////////////////////////////////////
Event :: Event(timestamp t, void* pf, void* po, long_integer li)
              : when(t), p_fun(pf), p_obj(po), li_param(li) {
  id = event_count++;
  li_param_flag = true;
  active = true;
}

////////////////////////////////////////////////////////////////////////////////
// Event::go                                                                  //
//                                                                            //
// perform event                                                              //
////////////////////////////////////////////////////////////////////////////////
int Event::go() {
  if (!active) {
    return 0;
  }

  if (li_param_flag) {
    (ptr2func_twopars_li(p_fun))(p_obj,li_param);
  } else if (p_param) {
    (ptr2func_twopars(p_fun))(p_obj, p_param);
  } else if (p_obj) {
    (ptr2func_onepar(p_fun))(p_obj);
  } else if (p_fun) {
    (ptr2func(p_fun)) ();
  } else {
    throw my_exception (EVENT,id, "invalid event");
  }

  active = false;
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Event:: comparison operators                                               //
////////////////////////////////////////////////////////////////////////////////

bool Event::operator< (Event e) const {
     return when < e.when;
}

bool Event::operator<= (Event e) const {
     return when <= e.when;
}

bool Event::operator> (Event e) const {
     return when > e.when;
}

bool Event::operator>= (Event e) const {
     return when >= e.when;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Scheduler                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Scheduler::init                                                            //
////////////////////////////////////////////////////////////////////////////////
void Scheduler::init () {
  present = timestamp(0);

  // clear events_list
  while(!empty()) pop();
}

////////////////////////////////////////////////////////////////////////////////
// Scheduler::schedule                                                        //
////////////////////////////////////////////////////////////////////////////////
long_integer Scheduler::schedule(Event e) {

    if (e.get_time() < now())
      throw(my_exception(EVENT,e.get_id(),"scheduling in the past"));

    push(e);
    return e.get_id();
}

////////////////////////////////////////////////////////////////////////////////
// Scheduler::remove                                                          //
////////////////////////////////////////////////////////////////////////////////
void Scheduler :: remove (long_integer event_id) {
  vector<Event>::iterator it = find_if(c.begin(), c.end(),
                                       same_event(event_id));

  if (it != c.end()) it->deactivate();
}

////////////////////////////////////////////////////////////////////////////////
void Scheduler :: remove (void* pf, void* po) {
  vector<Event>::iterator it = find_if(c.begin(), c.end(), same_event(pf,po));

  if (it != c.end()) {
    it->deactivate();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Scheduler::run                                                             //
////////////////////////////////////////////////////////////////////////////////
void Scheduler :: run (timestamp tmax) {

Event next;

  while (!empty()) {
    // get next event
    next = top();
    pop();

    present = next.get_time();

    if (present > tmax) return;
    next.go();
  }
  throw(my_exception(GENERAL,"Scheduler is empty"));
}

