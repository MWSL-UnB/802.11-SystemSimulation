
#include "DataStatistics.h"
#include "mymath.h"
#include "math.h"
#include "myexception.h"

////////////////////////////////////////////////////////////////////////////////
// student_ppf                                                                //
//                                                                            //
// calculates the percent point function (inverse of the cdf) for student's t //
// distribution at a a given probability 'p' for 'nu' degrees of freedom.     //
// 'nu' must be strictly positive and 0 < p < 1.                              //
//                                                                            //
// Implementation algorithm translated from FORTRAN code in DATAPAC           //
// http://gams.nist.gov/serve.cgi/ModuleComponent/2732/Documentation/ITL/tppf //
////////////////////////////////////////////////////////////////////////////////
double student_ppf(double p, int nu) {

  const double b21 = .25;
  const double b31 = .01041666666667;
  const double b32 = 5.0;
  const double b33 = 16.0;
  const double b34 = 3.0;
  const double b41 = 0.00260416666667;
  const double b42 = 3.0;
  const double b43 = 19.0;
  const double b44 = 17.0;
  const double b45 = -15.0;
  const double b51 = 0.00001085069444;
  const double b52 = 79.0;
  const double b53 = 776.0;
  const double b54 = 1482.0;
  const double b55 = -1920.0;
  const double b56 = -945.0;
  const int maxit = 5;
  
  if (p <= 0 || p >= 1.0 || nu < 1) {
    throw(my_exception("invalid argument in function student_ppf"));
    return 0;
  }
    
  if (nu == 1) {
  
    double arg = M_PI * p;

    return -cos(arg) / sin(arg);

  } else if (nu == 2) {

    double term1 = M_SQRT2 / 2.0;
    double term2 = 2.0 * p - 1.0;
    double term3 = sqrt(p*(1.0-p));
    
    return term1 * term2 / term3;

  } else {

    double ppfn = norm_ppf(p);
    double d1 = ppfn;
    double d3 = ppfn * ppfn * ppfn;
    double d5 = d3 * ppfn * ppfn;
    double d7 = d5 * ppfn * ppfn;
    double d9 = d7 * ppfn * ppfn;
    
    double term1 = d1;
    double term2 = b21 * (d3 + d1) / double(nu);    
    double term3 = b31 * (b32*d5 + b33*d3 + b34*d1) / double(nu*nu);
    double term4 = b41 * (b42*d7 + b43*d5 + b44*d3 + b45*d1) / double(nu*nu*nu);
    double term5 = b51 * (b52*d9 + b53*d7 + b54*d5 + b55*d3 + b56*d1)
                   / double(nu*nu*nu*nu);

    double ppf = term1 + term2 + term3 + term4 + term5;
    
    if (nu == 3) {

      double con = M_PI * (p - .5);
      double arg = ppf / sqrt(double(nu));
      double z = atan(arg);
      double s, c;
      for (int i = 1; i <= maxit; ++i) {
        s = sin(z);
        c = cos(z);
        z -= (z + s*c - con) / (2.0*c*c);
      }

      ppf = sqrt(double(nu)) * s / c;
      
    } else if (nu == 4) {
    
      double con = 2.0 * (p - .5);    
      double arg = ppf / sqrt(double(nu));
      double z = atan(arg);

      double s, c;      
      for (int i = 1; i <= maxit; ++i) {
        s = sin(z);
        c = cos(z);
        z -= ((1.0 + .5*c*c) * s - con) / (1.5*c*c*c);
      }

      ppf = sqrt(double(nu)) * s / c;
      
    } else if (nu == 5) {

      double con = M_PI * (p - .5);    
      double arg = ppf / sqrt(double(nu));
      double z = atan(arg);
      
      double s, c;
      for (int i = 1; i <= maxit; ++i) {
        s = sin(z);
        c = cos(z);
        z -= (z + (c + (2.0/3.0)*c*c*c) * s - con) / ((8.0/3.0)*c*c*c*c);
      }
      ppf = sqrt(double(nu)) * s / c;    
      
    } else if (nu == 6) {    
     
      double con = 2.0 * (p - .5);    
      double arg = ppf / sqrt(double(nu));
      double z = atan(arg);
      
      double s, c;
      for (int i = 1; i <= maxit; ++i) {
        s = sin(z);
        c = cos(z);
        z -= ((1.0 + .5*c*c + .375*c*c*c*c) * s - con) / ((15.0/8.0)*c*c*c*c*c);
      }

      ppf = sqrt(double(nu)) * s / c;
    
    }
    
    return ppf;
  }       
}

////////////////////////////////////////////////////////////////////////////////
// norm_ppf                                                                   //
// calculates the percent point function (inverse of the cdf) for the normal  //
// distribution at a a given probability 'p' (0 < p < 1).                     //
//                                                                            //
// Implementation algorithm translated from FORTRAN code in DATAPAC           //
// http://gams.nist.gov/serve.cgi/ModuleComponent/2674/Documentation/ITL      //
//                                                                  /normppf  //
////////////////////////////////////////////////////////////////////////////////
double norm_ppf(double p) {

  const double p0 = -0.322232431088;
  const double p1 = -1.0;
  const double p2 = -0.342242088547;
  const double p3 = -0.204231210245e-1;
  const double p4 = -0.453642210148e-4;
  const double q0 =  0.993484626060e-1;
  const double q1 =  0.588581570495;
  const double q2 =  0.531103462366;
  const double q3 =  0.103537752850;
  const double q4 =  0.38560700634e-2;
  
  if (p <= 0 || p >= 1.0) {
    throw(my_exception("invalid argument in function norm_ppf"));
    return 0;
  }
  
  if (p == .5) return 0;
  
  double r = p;
  if (p > .5) r = 1.0 - r;
  double t = sqrt(-2.0*log(r));
  double num = ((((t*p4 + p3) * t + p2) * t + p1) * t + p0);
  double den = ((((t*q4 + q3) * t + q2) * t + q1) * t + q0);
  double ppf = t + num/den;
  if (p < .5) ppf = - ppf;
  
  return ppf;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class DataStatistics                                                       //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DataStatistics constructor                                                 //
////////////////////////////////////////////////////////////////////////////////
DataStatistics::DataStatistics() {
  sum_x = 0;
  sum_x2 = 0;
  n_samples = 0;
}

////////////////////////////////////////////////////////////////////////////////
// DataStatistics::new_sample                                                 //
//                                                                            //
// collects a new sample 's'                                                  //
////////////////////////////////////////////////////////////////////////////////
void DataStatistics::new_sample(double s) {
  sum_x += s;
  sum_x2 += s*s;
  ++n_samples;
}

////////////////////////////////////////////////////////////////////////////////
// DataStatistics::mean                                                       //
//                                                                            //
// returns the average value of the samples                                   //
////////////////////////////////////////////////////////////////////////////////
double DataStatistics::mean() const {
  return sum_x / n_samples;
}

////////////////////////////////////////////////////////////////////////////////
// DataStatistics::confidence_interval                                        //
//                                                                            //
// returns the width of the confidence interval for a given confidence        //
////////////////////////////////////////////////////////////////////////////////
double DataStatistics::confidence_interval(double confidence) const {
  // calculate unbiased estimate of standard variation
  int N = n_samples - 1;
  
  if (N<1) return HUGE_VAL;
  
  double s = sqrt(sum_x2/N - sum_x*sum_x/double(n_samples*N));
  
  double t = student_ppf(1.0-(1.0-confidence)/2.0, N);
  
  return s*t;
}

////////////////////////////////////////////////////////////////////////////////
// DataStatistics::reset                                                      //
////////////////////////////////////////////////////////////////////////////////
void DataStatistics::reset() {
  sum_x = 0;
  sum_x2 = 0;
  n_samples = 0;  
}

