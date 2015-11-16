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

