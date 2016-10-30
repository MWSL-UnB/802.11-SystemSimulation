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
#include <stdlib.h>
#include <iostream>

using namespace std;

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

/*
 FFT/IFFT routine.

 Inputs:
	indata[] : array of complex* data points of size 2*NFFT+1.
		indata[0] is unused,
		* the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
			indata[2*n+1] = real(x(n))
			indata[2*n+2] = imag(x(n))
		if length(Nx) < NFFT, the remainder of the array must be padded with zeros

	nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
	isign:  if set to 1,
				computes the forward FFT
			if set to -1,
				computes Inverse FFT - in this case the output values have
				to be manually normalized by multiplying with 1/NFFT.
 Outputs:
	data[] : The FFT or IFFT results are stored in data

 Algorithm adapted from
    "Numerical Methods in C",
    Press, Teukolsky, Vetterling, Flannery
*/

valarray<double> four1(valarray<double> indata, int nn, int isign)
		{
	int n, mmax, m, j, istep, i;
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi;
	valarray<double> data;

	data.resize(indata.size(),0.0);
	data = indata;

	n = nn << 1;
	j = 1;
	for (i = 1; i < n; i += 2) {
		if (j > i) {
			tempr = data[j];     data[j] = data[i];     data[i] = tempr;
			tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
		}
		m = n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax = 2;
	while (n > mmax) {
		istep = 2*mmax;
		theta = TWOPI/(isign*mmax);
		wtemp = sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 1; m < mmax; m += 2) {
			for (i = m; i <= n; i += istep) {
				j =i + mmax;
				tempr = wr*data[j]   - wi*data[j+1];
				tempi = wr*data[j+1] + wi*data[j];
				if(j >= 2*nn) cout << " " << nn;
				data[j]   = data[i]   - tempr;
				data[j+1] = data[i+1] - tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr = (wtemp = wr)*wpr - wi*wpi + wr;
			wi = wi*wpr + wtemp*wpi + wi;
		}
		mmax = istep;
	}

	return data;
		}

double invraisedcos(double t, double W, double rollof) {
	double y;
	double phi = PI*W*t;

	if(abs(phi) < 1.0e-8) y = 1;
	else y = sin(phi)/phi;

	y *= cos(rollof*phi)/(1 - 4*pow((rollof*W*t),2));

	return y;

}
