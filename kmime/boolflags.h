#ifndef __KMIME_BOOLFLAGS_H__
#define __KMIME_BOOLFLAGS_H__

/** This class stores boolean values in single bytes.
    It provides a similiar functionality as QBitArray
    but requires much less memory.
    We use it to store the flags of an article
    @internal
*/
class BoolFlags {

public:
  BoolFlags()       { clear(); }
  ~BoolFlags()      {}

  void set(unsigned int i, bool b=true);
  bool get(unsigned int i);
  void clear()            { bits[0]=0; bits[1]=0; }
  unsigned char *data()   { return bits; }

protected:
  unsigned char bits[2];  //space for 16 flags
};

#endif // __KMIME_BOOLFLAGS_H__
