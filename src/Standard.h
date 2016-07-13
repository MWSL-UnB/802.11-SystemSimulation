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

public:
	static void set_standard(dot11_standard st);
	static dot11_standard get_standard();
	static transmission_mode get_maxMCS();
};

#endif /* STANDARD_H_ */
