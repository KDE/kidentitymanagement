#include "boolflags.h"

void BoolFlags::set(unsigned int i, bool b)
{
  if(i>15) return;

  unsigned char p; //bitmask
  int n;

  if(i<8) { //first byte
    p=(1 << i);
    n=0;
  }
  else { //second byte
    p=(1 << i-8);
    n=1;
  }

  if(b)
    bits[n] = bits[n] | p;
  else
    bits[n] = bits[n] & (255-p);
}


bool BoolFlags::get(unsigned int i)
{
  if(i>15) return false;

  unsigned char p; //bitmask
  int n;

  if(i<8) { //first byte
    p=(1 << i);
    n=0;
  }
  else { //second byte
    p=(1 << i-8);
    n=1;
  }

  return ( (bits[n] & p)>0 );
}


