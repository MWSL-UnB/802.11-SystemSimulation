#ifndef _random_h
#define _random_h 1

#include <vector>

///////////////////////////////////////////////////////////////////////
// class random
//
// pseudo-random numbers generator
//
// random numbers are generated according to the Mersenne-Twister
// method.
// http://www.math.keio.ac.jp/~matumoto/emt.html
//
// Usage:
//    Random numbers are obtained by calling member functions of a
//    random object.
//
//    Generator is initialised upon instanciation depending on
//    constructor parameters. If no parameter is given, seed is
//    chosen 'randomly' based on processor time.
//////////////////////////////////////////////////////////////////////

class random{
  unsigned long *mt; // the array for the state vector
  int mti;

  // the following variables are employed by normal distribution only
  bool norm_flag;
  double norm_aux;

public:
  /////////////////
  // constructors
  /////////////////
  random(); // creates new instance using 'random' seed obtained from
            // processor time
  random(unsigned long s);  // creates new instance using integer seed s
  random(unsigned long *s); // creates new instance using an integer
                            // array of dimension 624 for initialisation

  ///////////////
  // destructor
  ///////////////
  ~random();

  /////////////////////
  // member functions
  /////////////////////

  // redefine seeds
  void seed(unsigned long s); // using integer seed
  void seed(unsigned long *s); // using integer array (dimension 624)

  // returns random integer n with uniform discrete distribution,
  // 0 <= n <= ULONG_MAX
  unsigned long rand();

  // returns random integer n with uniform discrete distribution,
  // a <= n <= b
  int discrete_uniform(int a, int b);

  // returns a random integer n chosen from vector v
  // n belongs to v
  int from_vec(std::vector<int> v);

  // returns random float x with uniform distribution,
  // a <= x < b
  double uniform(double a=0, double b=1);

  // returns random float x with normal distribution, mean m, and
  // standard deviation sigma
  double normal(double m=0, double sigma=1);

  // returns random float x with exponential distribution, parameter
  // lambda, mean is 1/lambda, variance is 1/lambda^2
  double exponential(double lambda=1);
};

#endif
