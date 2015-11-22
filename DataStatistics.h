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

#ifndef data_Statistics_h
#define data_Statistics_h 1


////////////////////////////////////////////////////////////////////////////////
// class DataStatistics                                                       //
//                                                                            //
// collects data samples and calculates their statistics                      //
////////////////////////////////////////////////////////////////////////////////
class DataStatistics {
double sum_x;  // sum of samples
double sum_x2; // sum of squared samples
int n_samples; // number of samples

public:

DataStatistics();

void reset();

void new_sample(double s);
// collects a new sample 's'

double mean() const;
// returns the average value of the samples

double sum() const {return sum_x;}
// returns the summation of all samples

double confidence_interval(double confidence) const;
// returns the width of the confidence interval for a given confidence
// (0 < confidence < 1)
// NOTICE: the confidence interval assumes a normal distribution, it is likely 
//         to be too pessimistic for other distributions
};
////////////////////////////////////////////////////////////////////////////////

double student_ppf(double p, int nu);
// calculates the percent point function (inverse of the cdf) for student's t
// distribution at a a given probability 'p' for 'nu' degrees of freedom.
// 'nu' must be strictly positive and 0 < p < 1

double norm_ppf(double p);
// calculates the percent point function (inverse of the cdf) for the normal
// distribution at a a given probability 'p' (0 < p < 1).

#endif
