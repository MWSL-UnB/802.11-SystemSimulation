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

#ifndef _myexception_h
#define _myexception_h 1

#include "long_integer.h"
#include <string>

using namespace std;


typedef enum {CONFIG,
              EVENT,
              GENERAL,
              OPENFILE,
              SYNTAX,
              TS_OVERFLOW} exception_type;


class my_exception{
  exception_type exc;
  string comp_str;
  long_integer li;
public:
  my_exception () {exc = GENERAL;}
  my_exception (string s) {exc = GENERAL; comp_str = s;} 
  my_exception (exception_type e) : exc(e) {}
  my_exception (exception_type e, string s) {exc = e; comp_str = s;}
  my_exception (exception_type e, long_integer i, string s = "") {
    exc = e; li = i; comp_str = s;}

  string what();
};

#endif
