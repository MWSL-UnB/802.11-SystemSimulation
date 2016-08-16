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

#ifndef STANDARD_H_
#define STANDARD_H_ 1

#include <iostream>

#include "Packet.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Error model coefficient number
////////////////////////////////////////////////////////////////////////////////
const int n_coeff = 5;
const int n_coeff_high = 2;

////////////////////////////////////////////////////////////////////////////////
// Standard to be simulated
////////////////////////////////////////////////////////////////////////////////
typedef enum{
	dot11, //dummy
	dot11a,
	dot11n,
	dot11ac,
	dot11ah
}dot11_standard;

ostream& operator<< (ostream& os, const dot11_standard& st);
istream& operator>> (istream& is, dot11_standard& st);

////////////////////////////////////////////////////////////////////////////////
// class Standard                                                             //
////////////////////////////////////////////////////////////////////////////////
class Standard {
private:
	static dot11_standard currentStd;
	static transmission_mode maxMCS;
	static double symbol_period;      //OFDM symbol period
	static bool short_guard;

	// Error model constants
	static double min_thresh_a[8];
	static double max_thresh_a[8];
	static double coeff_a[8][5];
	static double coeff_high_a[8][2];

	static double min_thresh_n[8];
	static double max_thresh_n[8];
	static double coeff_n[8][5];
	static double coeff_high_n[8][2];

	static double min_thresh_ac_ah[10];
	static double max_thresh_ac_ah[10];
	static double coeff_ac_ah[10][5];
	static double coeff_high_ac_ah[10][2];

public:
	static void set_standard(dot11_standard st, bool sg);
	static dot11_standard get_standard();
	static transmission_mode get_maxMCS();
	static double get_symbol_period();
	static double get_min_thresh(int idx);
	static double get_max_thresh(int idx);
	static double get_coeff(int idx, int i);
	static double get_coeff_high(int idx, int i);

	static double tx_mode_to_double(transmission_mode tm);
	static unsigned txMode_bits_per_symbol(transmission_mode tm);
};

#endif /* STANDARD_H_ */
