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

#include "myexception.h"

#include <string>
#include <sstream>

string my_exception::what () {
string s;

  switch (exc) {
    case CONFIG:
      return ("Configuration error");

    case EVENT: {
      ostringstream sstr;
      sstr << "Error in event " << li;
      if (comp_str.size()) sstr << ", " << comp_str;
      return sstr.str();
    }

    case OPENFILE:
      return (comp_str + " could not be opened!");

    case SYNTAX: {
      ostringstream sstr;
      sstr << "Syntax error in line " << li;
      return sstr.str();
    }

    case TS_OVERFLOW:
      return  "Timestamp overflow";

    default:
      return comp_str;
  }

}

