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

#include <time.h>
#include <limits.h>
#include <math.h>

#include "random.h"


/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

/////////////////
// Constructors
/////////////////
random :: random () {
  mt = new unsigned long[N];
  mti = N;
  norm_flag = false;

  seed (clock());
}

random :: random (unsigned long s) {
  mt = new unsigned long[N];

  seed (s);
}

random :: random (unsigned long* s) {
  mt = new unsigned long[N];

  seed (s);
}


///////////////
// Destructor
///////////////
random::~random () {
 delete [] mt;
}

//////////////////
// seed
// Redefines seed
//////////////////
void random::seed (unsigned long s) {

    mti = N;
    norm_flag = false;

    for (int i=0;i<N;i++) {
         mt[i] = s & 0xffff0000;
         s = 69069 * s + 1;
         mt[i] |= (s & 0xffff0000) >> 16;
         s = 69069 * s + 1;
    }

}

void random::seed (unsigned long *s) {
  mti = N;
  norm_flag = false;

  for (int i=0;i<N;i++)
    mt[i] = s[i];
}

///////////////////////////////////////////////////////////
// rand
// generates integer-valued pseudorandom number
// this function is called by all the other distributions
///////////////////////////////////////////////////////////
unsigned long random::rand()
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }
  
    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y; 
}

/////////////////////////////////////////////////////
// uniform
// generates random float with uniform distribution
/////////////////////////////////////////////////////
double random::uniform (double a, double b) {
  double x;
  unsigned long i;

  do {
    i = rand();
  } while (i == ULONG_MAX);

  x = double(i)/double(ULONG_MAX);

  x = a+x*(b-a);

  return x;
}

/////////////////////////////////////////////////////
// normal
// generates random float with normal distribution
// using Box-Muller transformation
/////////////////////////////////////////////////////

double random::normal(double m, double sigma){
  double x, w;

  if (!norm_flag) {
    double u1, u2;
    do {
      u1 = uniform(-1,1);
      u2 = uniform(-1,1);
    } while ((w=u1*u1+u2*u2)>=1 || w==0);

    w = sqrt(-2*log(w)/w);
    x = u1*w;
    norm_aux = u2*w;
  }
  else {
    x = norm_aux;
  }
  norm_flag = !norm_flag;

  return x*sigma+m;
}

/////////////////////////////////////////////////////
// normal
// generates random float with exponential distribution
/////////////////////////////////////////////////////
double random::exponential(double lambda){

double x;

  do {
    x = uniform();
  } while (x==0);

  return -log(x)/lambda;
}

/////////////////////////////////////////////////////////////////
// returns random integer n with uniform discrete distribution,
// a <= n <= b
/////////////////////////////////////////////////////////////////
int random::discrete_uniform(int a, int b) {
  return (a+rand()%(b-a+1));
}

