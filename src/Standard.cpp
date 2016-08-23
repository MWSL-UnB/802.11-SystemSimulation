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
channel_bandwidth maxBand = MHz;

// Standard error model polynomials
//802.11a
double Standard::min_thresh_a[8] = {-2.5103, 0.75061,  0.5000,  3.7609,
        5.5103, 9.2712 , 12.5206, 14.5321};
double Standard::max_thresh_a[8] = { 1.9897,  5.2506,  5.0000,  8.2609,
        10.5103, 14.7712, 18.5206, 20.0321};
double Standard::coeff_a[8][5] = {
        {  -2.2353000, -1.0721000,-0.1708900, 0.0243860, 0.0096656},
        {  -0.3624500, -0.2937100,-0.0011057,-0.0408500, 0.0038022},
        {  -0.4517200, -0.3560888, 0.0627930,-0.0651410, 0.0064799},
        {  -0.3082200, -0.2063400, 0.1547100,-0.0389730, 0.0018157},
        {   2.6965000, -1.9353000, 0.4736600,-0.0509360, 0.0016224},
        {  34.8692000,-13.9070000, 2.0328000,-0.1283100, 0.0028499},
        {  93.9622000,-26.7075000, 2.8106000,-0.1290100, 0.0021372},
        {-120.1972000, 26.3772000,-2.1564000, 0.0787190,-0.0011189}};
double Standard::coeff_high_a[8][2] = { {-2.3974,-1.1580}, { 2.8250,-1.4824},
        { 2.1138,-1.3738}, { 7.7079,-1.5347},
        { 9.2576,-1.3244}, {11.3789,-1.1004},
        {14.6479,-1.0454}, {20.0742,-1.2278} };

//802.11n
/*double Standard::min_thresh_n[8] = {-7.381166,-4.370866,-0.609954,0.639434,
		4.400346,7.649733,9.161259,10.618834};
double Standard::max_thresh_n[8] = {-1.030000,2.150000,4.890000,7.140000,
		11.900000,16.150000,17.160000,19.620000};
double Standard::coeff_n[8][5] = {
		{0.477069,0.539243,0.193293,0.022534,0.000841},
		{0.067758,-0.092143,0.025659,0.003090,-0.000754},
		{0.447849,-0.036744,-0.035149,-0.004960,0.002338},
		{0.339689,0.210892,-0.108324,0.013670,-0.000499},
		{-1.086713,0.677080,-0.080902,0.000705,0.000171},
		{-13.104728,4.680099,-0.578657,0.030269,-0.000574},
		{-10.810128,3.108263,-0.296372,0.011188,-0.000136},
		{-19.609097,5.025594,-0.449306,0.016866,-0.000225}};
double Standard::coeff_high_n[8][2] = { {-4.629642,-1.160811}, {-0.568381,-1.335611},
		{3.719234,-1.379019},{-0.652125,-0.465231},
		{-0.652125,-0.465231}, {17.686547,-1.353782},
		{11.835825,-0.901625}, {20.245745,-1.245957}};*/
double Standard::min_thresh_n[8] = {-2.5103, 0.5000, 3.7609, 5.5103, 9.2712,
		12.5206,14.5321,11.6188};
double Standard::max_thresh_n[8] = { 1.9897,  5.0000,  8.2609, 10.5103, 14.7712,
		18.5206, 20.0321, 21.5000};
double Standard::coeff_n[8][5] = {
		{  -2.2353000, -1.0721000,-0.1708900, 0.0243860, 0.0096656},
		{  -0.4517200, -0.3560888, 0.0627930,-0.0651410, 0.0064799},
		{  -0.3082200, -0.2063400, 0.1547100,-0.0389730, 0.0018157},
		{   2.6965000, -1.9353000, 0.4736600,-0.0509360, 0.0016224},
		{  34.8692000,-13.9070000, 2.0328000,-0.1283100, 0.0028499},
		{  93.9622000,-26.7075000, 2.8106000,-0.1290100, 0.0021372},
		{-120.1972000, 26.3772000,-2.1564000, 0.0787190,-0.0011189},
		{ -42.9748720, 11.0385340,-1.0249930, 0.0411790,-0.0006080}};
double Standard::coeff_high_n[8][2] = {
		{-2.3974,-1.1580}, { 2.1138,-1.3738}, { 7.7079,-1.5347},
		{ 9.2576,-1.3244}, {11.3789,-1.1004}, {14.6479,-1.0454},
		{20.0742,-1.2278}, {21.2886,-1.2977}};

// 802.11ac and 802.11ah
double Standard::min_thresh_ac_ah[10] = {-2.5103, 0.5000, 3.7609, 5.5103, 9.2712,
		12.5206,14.5321,11.6188,15.4106, 0};
double Standard::max_thresh_ac_ah[10] = { 1.9897,  5.0000,  8.2609, 10.5103, 14.7712,
		18.5206, 20.0321, 21.5000, 22.5000, 0};
double Standard::coeff_ac_ah[10][5] = {{  -2.2353000, -1.0721000,-0.1708900, 0.0243860, 0.0096656},
		{  -0.4517200, -0.3560888, 0.0627930,-0.0651410, 0.0064799},
		{  -0.3082200, -0.2063400, 0.1547100,-0.0389730, 0.0018157},
		{   2.6965000, -1.9353000, 0.4736600,-0.0509360, 0.0016224},
		{  34.8692000,-13.9070000, 2.0328000,-0.1283100, 0.0028499},
		{  93.9622000,-26.7075000, 2.8106000,-0.1290100, 0.0021372},
		{-120.1972000, 26.3772000,-2.1564000, 0.0787190,-0.0011189},
		{ -42.9748720, 11.0385340,-1.0249930, 0.0411790,-0.0006080},
		{ -159.279240, 33.9061380,-2.6679000, 0.0922430,-0.0011850},
		{    0.000000, 0.000000,  0.000000,   0.000000,  0.000000}};
double Standard::coeff_high_ac_ah[10][2] = {{-2.3974,-1.1580}, { 2.1138,-1.3738}, { 7.7079,-1.5347},
		{ 9.2576,-1.3244}, {11.3789,-1.1004}, {14.6479,-1.0454},
		{20.0742,-1.2278}, {21.2886,-1.2977}, {18.1224,-0.9725},
		{0,0}};

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
double Standard::get_min_thresh(int idx) {
	if(currentStd == dot11a) return min_thresh_a[idx];
	else if(currentStd == dot11n) return min_thresh_n[idx];
	else return min_thresh_ac_ah[idx];
}
double Standard::get_max_thresh(int idx) {
	if(currentStd == dot11a) return max_thresh_a[idx];
	else if(currentStd == dot11n) return max_thresh_n[idx];
	else return max_thresh_ac_ah[idx];
}
double Standard::get_coeff(int idx, int i) {
	if(currentStd == dot11a) return coeff_a[idx][i];
	else if(currentStd == dot11n) return coeff_n[idx][i];
	else return coeff_ac_ah[idx][i];
}
double Standard::get_coeff_high(int idx, int i) {
	if(currentStd == dot11a) return coeff_high_a[idx][i];
	else if(currentStd == dot11n) return coeff_high_n[idx][i];
	else return coeff_high_ac_ah[idx][i];
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
		case MCS0: return 6.5;
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

////////////////////////
// output operator<<  //
////////////////////////
ostream& operator<< (ostream& os, const channel_bandwidth& bw) {
  switch(bw) {
    case MHz : return os << "MHz";
    case MHz20 : return os << "20 MHz";
    case MHz40 : return os << "40 MHz";
    case MHz80 : return os << "80 MHz";
    case MHz160: return os << "160 MHz";
    default: return os << "unknown bandwidth.";
  }
}

///////////////////////
// input operator >> //
///////////////////////
istream& operator>> (istream& is, channel_bandwidth& bw) {
  string str;
  is >> str;

  if (str == "20MHz") bw = MHz20;
  else if (str == "40MHz") bw = MHz40;
  else if (str == "80MHz") bw = MHz80;
  else if (str == "160MHz") bw = MHz160;
  else is.clear(ios::failbit);

  return is;
}

