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

#include "Standard.h"
#include "Packet.h"

// Static member variables need to be defined outside the class
dot11_standard Standard::currentStd = dot11;
transmission_mode Standard::maxMCS = MCS;

//////////////////////////////////
// Standard setters and getters //
//////////////////////////////////
void Standard::set_standard(dot11_standard st) {
	currentStd = st;

	switch(st) {
	case dot11a : maxMCS = MCS7; break;
	case dot11n : maxMCS = MCS7; break;
	case dot11ac: maxMCS = MCS9; break;
	case dot11ah: maxMCS = MCS9; break;
	default : maxMCS = MCS;
	}

}
dot11_standard Standard::get_standard() {
	return currentStd;
}
transmission_mode Standard::get_maxMCS(){
	return maxMCS;
}

////////////////////////
// output operator<<  //
////////////////////////
ostream& operator<< (ostream& os, const dot11_standard& st) {
  switch(st) {
    case dot11a : return os << "802.11a";
    case dot11n : return os << "802.11n";
    case dot11ac: return os << "802.11ac";
    case dot11ah: return os << "802.11ah";
    default: return os << "unknown standard.";
  }
}

///////////////////////
// input operator >> //
///////////////////////
istream& operator>> (istream& is, dot11_standard& st) {
  string str;
  is >> str;

  if (str == "802.11a") st = dot11a;
  else if (str == "802.11n") st = dot11n;
  else if (str == "802.11ac") st = dot11ac;
  else if (str == "802.11ah") st = dot11ah;
  else is.clear(ios::failbit);

  return is;
}

