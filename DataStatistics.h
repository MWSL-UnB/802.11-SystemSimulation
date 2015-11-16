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
