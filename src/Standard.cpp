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
#include "myexception.h"
#include "timestamp.h"

// Static member variables need to be defined outside the class
dot11_standard Standard::currentStd = dot11;
transmission_mode Standard::maxMCS = MCS;
double Standard::symbol_period = 4e-6;

//////////////////////////////////
// Standard setters and getters //
//////////////////////////////////
void Standard::set_standard(dot11_standard st) {
	currentStd = st;

	switch(st) {
	case dot11a : maxMCS = MCS7; break;
	case dot11n : maxMCS = MCS7; break;
	case dot11ac: maxMCS = MCS8; break;
	case dot11ah: maxMCS = MCS8; break;
	default : maxMCS = MCS;
	}

	if(st == dot11ah) symbol_period = 40e-6;
	else symbol_period = 4e-6;

}
dot11_standard Standard::get_standard() {
	return currentStd;
}
transmission_mode Standard::get_maxMCS(){
	return maxMCS;
}
double Standard::get_symbol_period(){
	return symbol_period;
}

///////////////////////
// tx_mode_to_double //
///////////////////////
double Standard::tx_mode_to_double (transmission_mode tm) {
	switch(currentStd) {
	case dot11a: {
		switch(tm) {
		case MCS : return 0;
		case MCS0: return 6;
		case MCS1: return 9;
		case MCS2: return 12;
		case MCS3: return 18;
		case MCS4: return 24;
		case MCS5: return 36;
		case MCS6: return 48;
		case MCS7: return 54;
		default : throw (my_exception("MCS not supported by standard."));
		}
	}
	case dot11n: {
		switch(tm) {
		case MCS : return 0;
		case MCS0: return  6.5;
		case MCS1: return 13.0;
		case MCS2: return 19.5;
		case MCS3: return 26.0;
		case MCS4: return 39.0;
		case MCS5: return 52.0;
		case MCS6: return 58.5;
		case MCS7: return 65.0;
		default : throw (my_exception("MCS not supported by standard."));
		}
	}
	case dot11ac: {
		switch(tm) {
		case MCS : return 0;
		case MCS0: return 6.5;
		case MCS1: return 13.0;
		case MCS2: return 19.5;
		case MCS3: return 26.0;
		case MCS4: return 39.0;
		case MCS5: return 52.0;
		case MCS6: return 58.5;
		case MCS7: return 65.0;
		case MCS8: return 78.0;
		default : throw (my_exception("MCS not supported by standard."));
		}
	}
	case dot11ah: {
		switch(tm) {
		case MCS : return 0;
		case MCS0: return 0.65;
		case MCS1: return 1.30;
		case MCS2: return 1.95;
		case MCS3: return 2.60;
		case MCS4: return 3.90;
		case MCS5: return 5.20;
		case MCS6: return 5.85;
		case MCS7: return 6.50;
		case MCS8: return 7.80;
		default : throw (my_exception("MCS not supported by standard."));
		}
	}
	default: throw (my_exception("Undefined Standard."));
	}
}

////////////////////////////
// txMode_bits_per_symbol //
////////////////////////////
unsigned Standard::txMode_bits_per_symbol(transmission_mode tm) {
	switch(currentStd) {
	case dot11a: {
		switch (tm) {
		case MCS0 : return 24;
		case MCS1 : return 36;
		case MCS2 : return 48;
		case MCS3 : return 72;
		case MCS4 : return 96;
		case MCS5 : return 144;
		case MCS6 : return 192;
		case MCS7 : return 216;
		default   : return 0;
		} break;
	}
	case dot11n: {
		switch (tm) {
		case MCS0 : return 26;
		case MCS1 : return 52;
		case MCS2 : return 78;
		case MCS3 : return 104;
		case MCS4 : return 156;
		case MCS5 : return 208;
		case MCS6 : return 234;
		case MCS7 : return 260;
		default   : return 0;
		} break;
	}
	case dot11ac: {
		switch (tm) {
		case MCS0 : return 26;
		case MCS1 : return 52;
		case MCS2 : return 78;
		case MCS3 : return 104;
		case MCS4 : return 156;
		case MCS5 : return 208;
		case MCS6 : return 234;
		case MCS7 : return 260;
		case MCS8 : return 312;
		default   : return 0;
		} break;
	}
	case dot11ah: {
		switch (tm) {
		case MCS0 : return 26;
		case MCS1 : return 52;
		case MCS2 : return 78;
		case MCS3 : return 104;
		case MCS4 : return 156;
		case MCS5 : return 208;
		case MCS6 : return 234;
		case MCS7 : return 260;
		case MCS8 : return 312;
		default   : return 0;
		} break;
	}
	default: throw (my_exception("Undefined Standard."));
	}
	return 0;
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
    case dot11  : return os << "dummy standard";
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

