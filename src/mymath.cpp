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

#include "mymath.h"

#include <math.h>

////////////////////////////////////////////////////
// bessel_j0
//
// returns the J-Bessel function of order 0 J_0(x)
//
// algorithm from
//    "Numerical Methods in C",
//    Press, Teukolsky, Vetterling, Flannery
////////////////////////////////////////////////////
double bessel_j0 (double x){
  double ax;
  double ans;

  if ((ax=fabs(x)) < 8.0) {
    double y = x*x;
    double ans1 = 57568490574.0 + y*(-13362590354.0 + y*(651619640.7 +
                  y*(-11214424.18 + y*(77392.33017 + y*(-184.9052456)))));
    double ans2 = 57568490411.0 + y*(1029532985.0 + y*(9494680.718 +
                  y*(59272.64853 + y*(267.8532712 + y*1.0))));
    ans = ans1/ans2;
  } else {
    double z = 8.0/ax;
    double y = z*z;
    double xx = ax - 0.785398164;
    double ans1 = 1.0 + y*(-0.1098628627e-2 + y*(0.2734510407e-4 +
                  y*(-0.2073370639e-5 + y*0.2093887211e-6)));
    double ans2 = -0.1562499995e-1 + y*(0.1430488765e-3 +
                  y*(-0.6911147651e-5 + y*(0.7621095161e-6 -
                  y*0.934935152e-7)));
    ans = sqrt(0.636619772/ax)*(cos(xx)*ans1-z*sin(xx)*ans2);
  }
  return ans;
}
