/*
    kmime_codecs.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, US
*/

#include "kmime_codecs.h"
#include "kmime_util.h"

#include <kdebug.h>

#include <cassert>
#include <cstring>

using namespace KMime;

namespace KMime {

// forward declarations:
template <typename S, typename D>
class Base64Codec;
template <typename S, typename D>
class Rfc2047BEncodingCodec;
template <typename S, typename D>
class QuotedPrintableCodec;
template <typename S, typename D>
class Rfc2047QEncodingCodec;
template <typename S, typename D>
class Rfc2231EncodingCodec;

// global list of KMime::Codec's
template <typename S, typename D>
QAsciiDict< Codec<S,D> > Codec<S,D>::all( 11, false /* case-insensitive */);
#if defined(QT_THREAD_SUPPORT)
template <typename S, typename D>
QMutex Codec<S,D>::dictLock;
#endif

template <typename S, typename D>
void Codec<S,D>::fillDictionary() {

  all.setAutoDelete(true);

  all.insert( "base64", new Base64Codec<S,D>() );
  all.insert( "quoted-printable", new QuotedPrintableCodec<S,D>() );
  all.insert( "b", new Rfc2047BEncodingCodec<S,D>() );
  all.insert( "q", new Rfc2047QEncodingCodec<S,D>() );
  all.insert( "x-kmime-rfc2231", new Rfc2231EncodingCodec<S,D>() );

}

template <typename S, typename D>
Codec<S,D> * Codec<S,D>::codecForName( const char * name ) {
#if defined(QT_THREAD_SUPPORT)
  dictLock.lock(); // protect "all"
#endif
  if ( all.isEmpty() )
    fillDictionary();
  Codec<S,D> * codec = all[ name ];
#if defined(QT_THREAD_SUPPORT)
  dictLock.unlock();
#endif

  if ( !codec )
    kdWarning() << "Unknown codec \"" << name << "\" requested!" << endl;

  return codec;
}

template <typename S, typename D>
Codec<S,D> * Codec<S,D>::codecForName( const QCString & name ) {
  return codecForName( name.data() );
}

template <typename S, typename D>
bool Codec<S,D>::encode( S & scursor, const S & send,
			 D & dcursor, const D & dend, bool withCRLF ) const
{
  // get an encoder:
  Encoder<S,D> * enc = makeEncoder( withCRLF );
  assert( enc );

  // encode and check for output buffer overflow:
  while ( !enc->encode( scursor, send, dcursor, dend ) )
    if ( !(dcursor != dend) ) { // don't force operator== on iterators
      delete enc;
      return false; // not enough space in output buffer
    }

  // finish and check for output buffer overflow:
  while ( !enc->finish( dcursor, dend ) )
    if ( !(dcursor != dend) ) { // don't force operator== on iterators
      delete enc;
      return false; // not enough space in output buffer
    }

  // cleanup and return:
  delete enc;
  return true; // successfully encoded.
}

template <typename S, typename D>
bool Codec<S,D>::decode( S & scursor, const S & send,
			 D & dcursor, const D & dend, bool withCRLF ) const
{
  // get a decoder:
  Decoder<S,D> * dec = makeDecoder( withCRLF );
  assert( dec );

  // decode and check for output buffer overflow:
  while ( !dec->decode( scursor, send, dcursor, dend ) )
    if ( !(dcursor != dend) ) { // don't force operator== on iterators
      delete dec;
      return false; // not enough space in output buffer
    }

  // finish and check for output buffer overflow:
  while ( !dec->finish( dcursor, dend ) )
    if ( !(dcursor != dend) ) { // don't force operator== on iterators
      delete dec;
      return false; // not enough space in output buffer
    }

  // cleanup and return:
  delete dec;
  return true; // successfully encoded.
}


//
// Base64{De,En}coder
//

template <typename S, typename D>
class Base64Decoder : public Decoder<S,D> {
  uchar mOutbits;
  uint mStepNo : 2;
  bool mSawPadding : 1;
  const bool mWithCRLF : 1;

protected:
  friend class Base64Codec<S,D>;
  Base64Decoder( bool withCRLF=false )
    : Decoder<S,D>( withCRLF ), mOutbits(0), mStepNo(0),
      mSawPadding(false), mWithCRLF( withCRLF ) {}

public:
  virtual ~Base64Decoder() {}

  bool decode( S & scursor, const S & send,
	       D & dcursor, const D & dend );
  // ### really needs no finishing???
  bool finish( D & /*dcursor*/, const D & /*dend*/ ) { return true; }
}; // class Base64Decoder

template <typename S, typename D>
class Rfc2047BEncodingEncoder; // forward decl.

template <typename S, typename D>
class Base64Encoder : public Encoder<S,D> {
  uchar mNextbits;
  uint mStepNo : 2;
  bool mInsideFinishing : 1;
  /** number of already written base64-quartets on current line */
  uint mWrittenPacketsOnThisLine : 5;
  const bool mWithCRLF : 1;

protected:
  friend class Rfc2047BEncodingCodec<S,D>;
  friend class Rfc2047BEncodingEncoder<S,D>;
  friend class Base64Codec<S,D>;
  Base64Encoder( bool withCRLF=false )
    : Encoder<S,D>(), mNextbits(0), mStepNo(0),
      mInsideFinishing(false), mWrittenPacketsOnThisLine(0),
      mWithCRLF( withCRLF ) {}

  bool generic_finish( D & dcursor, const D & dend, bool withLFatEnd );

public:
  virtual ~Base64Encoder() {}

  bool encode( S & scursor, const S & send,
	       D & dcursor, const D & dend );

  bool finish( D & dcursor, const D & dend );
}; // class Base64Encoder

template <typename S, typename D>
class Base64Codec : public Codec<S,D> {
protected:
  friend class Codec<S,D>;
  Base64Codec() : Codec<S,D>() {}

public:
  virtual ~Base64Codec() {}

  const char * name() const {
    return "base64";
  }

  Encoder<S,D> * makeEncoder( bool withCRLF=false ) const {
    return new Base64Encoder<S,D>( withCRLF );
  }

  Decoder<S,D> * makeDecoder( bool withCRLF=false ) const {
    return new Base64Decoder<S,D>( withCRLF );
  }

};

template <typename S, typename D>
class Rfc2047BEncodingEncoder : public Base64Encoder<S,D> {
protected:
  friend class Rfc2047BEncodingCodec<S,D>;
  Rfc2047BEncodingEncoder( bool withCRLF=false )
    : Base64Encoder<S,D>( withCRLF ) {};
public:
  bool finish( D & dcursor, const D & dend );
}; // class Rfc2047BEncodingEncoder

template <typename S, typename D>
class Rfc2047BEncodingCodec : public Base64Codec<S,D> {
protected:
  friend class Codec<S,D>;
  Rfc2047BEncodingCodec()
    : Base64Codec<S,D>() {}
  
public:
  virtual ~Rfc2047BEncodingCodec() {}

  const char * name() const { return "B"; }

  Encoder<S,D> * makeEncoder( bool withCRLF=false ) const {
    return new Rfc2047BEncodingEncoder<S,D>( withCRLF );
  }
};


//
// QuotedPrintableCodec
//

template <typename S, typename D>
class QuotedPrintableEncoder : public Encoder<S,D> {
  char mInputBuffer[16];
  char mOutputBuffer[8];
  uchar mCurrentLineLength; // 0..76
  uchar mAccu;
  uint mInputBufferReadCursor  : 4; // 0..15
  uint mInputBufferWriteCursor : 4; // 0..15
  uint mOutputBufferCursor     : 3; // 0..8
  enum {
    Never, AtBOL, Definitely
  } mAccuNeedsEncoding    : 2;
  bool mSawLineEnd        : 1;
  bool mSawCR             : 1;
  bool mFinishing         : 1;
  const bool mWithCRLF    : 1;
protected:
  friend class QuotedPrintableCodec<S,D>;
  QuotedPrintableEncoder<S,D>( bool withCRLF=false )
    : Encoder<S,D>(), mCurrentLineLength(0), mAccu(0),
      mInputBufferReadCursor(0), mInputBufferWriteCursor(0),
      mOutputBufferCursor(0), mAccuNeedsEncoding(Never),
      mSawLineEnd(false), mSawCR(false), mFinishing(false),
      mWithCRLF( withCRLF ) {}

  bool needsEncoding( uchar ch ) {
    return ( ch > '~' || ch < ' ' && ch != '\t' || ch == '=' );
  }
  bool fillInputBuffer( S & scursor, const S & send );
  bool processNextChar();
  void createOutputBuffer();
public:
  virtual ~QuotedPrintableEncoder() {}

  bool encode( S & scursor, const S & send,
	       D & dcursor, const D & dend );

  bool finish( D & dcursor, const D & dend );
};

template <typename S, typename D>
class QuotedPrintableDecoder : public Decoder<S,D> {
  const char mEscapeChar;
  char mBadChar;
  /** @p accu holds the msb nibble of the hexchar or zero. */
  uchar mAccu;
  /** @p insideHexChar is true iff we're inside an hexchar (=XY).
      Together with @ref mAccu, we can build this states:
      @li @p insideHexChar == @p false:
          normal text
      @li @p insideHexChar == @p true, @p mAccu == 0:
          saw the leading '='
      @li @p insideHexChar == @p true, @p mAccu != 0:
          saw the first nibble '=X'
   */
  bool mInsideHexChar   : 1;
  bool mFlushing        : 1;
  bool mExpectLF        : 1;
  bool mHaveAccu        : 1;
  const bool mQEncoding : 1;
  const bool mWithCRLF  : 1;
protected:
  friend class QuotedPrintableCodec<S,D>;
  friend class Rfc2047QEncodingCodec<S,D>;
  friend class Rfc2231EncodingCodec<S,D>;
  QuotedPrintableDecoder( bool withCRLF=false,
			  bool aQEncoding=false, char aEscapeChar='=' )
    : Decoder<S,D>(),
    mEscapeChar(aEscapeChar),
    mBadChar(0),
    mAccu(0),
    mInsideHexChar(false),
    mFlushing(false),
    mExpectLF(false),
    mHaveAccu(false),
    mQEncoding(aQEncoding),
    mWithCRLF( withCRLF ) {}
public:
  virtual ~QuotedPrintableDecoder() {}

  bool decode( S & scursor, const S & send,
	       D & dcursor, const D & dend );
  // ### really no finishing needed???
  bool finish( D &, const D & ) { return true; }
};

template <typename S, typename D>
class QuotedPrintableCodec : public Codec<S,D> {
protected:
  friend class Codec<S,D>;
  QuotedPrintableCodec() : Codec<S,D>() {}

public:
  virtual ~QuotedPrintableCodec() {}

  const char * name() const {
    return "quoted-printable";
  }

  Encoder<S,D> * makeEncoder( bool withCRLF=false ) const {
    return new QuotedPrintableEncoder<S,D>( withCRLF );
  }

  Decoder<S,D> * makeDecoder( bool withCRLF=false ) const {
    return new QuotedPrintableDecoder<S,D>( withCRLF );
  }
};

template <typename S, typename D>
class Rfc2047QEncodingEncoder : public Encoder<S,D> {
  uchar mAccu;
  uchar mStepNo;
  const char  mEscapeChar;
  const bool  mWithCRLF;
protected:
  friend class Rfc2047QEncodingCodec<S,D>;
  friend class Rfc2231EncodingCodec<S,D>;
  Rfc2047QEncodingEncoder( bool withCRLF=false, char aEscapeChar='=' )
    : Encoder<S,D>(),
      mAccu(0), mStepNo(0), mEscapeChar( aEscapeChar ),
      mWithCRLF( withCRLF ) {}

public:
  virtual ~Rfc2047QEncodingEncoder() {}

  bool encode( S & scursor, const S & send,
	       D & dcursor, const D & dend );
  bool finish( D & dcursor, const D & dend );
};


template <typename S, typename D>
class Rfc2047QEncodingCodec : public Codec<S,D> {
protected:
  friend class Codec<S,D>;
  Rfc2047QEncodingCodec() : Codec<S,D>() {}

public:
  virtual ~Rfc2047QEncodingCodec() {}

  const char * name() const {
    return "Q";
  }

  Encoder<S,D> * makeEncoder( bool withCRLF=false ) const {
    return new Rfc2047QEncodingEncoder<S,D>( withCRLF );
  }

  Decoder<S,D> * makeDecoder( bool withCRLF=false ) const {
    return new QuotedPrintableDecoder<S,D>( withCRLF, true );
  }
};


template <typename S, typename D>
class Rfc2231EncodingCodec : public Codec<S,D> {
protected:
  friend class Codec<S,D>;
  Rfc2231EncodingCodec() : Codec<S,D>() {}

public:
  virtual ~Rfc2231EncodingCodec() {}

  const char * name() const {
    return "x-kmime-rfc2231";
  }

  Encoder<S,D> * makeEncoder( bool withCRLF=false ) const {
    return new Rfc2047QEncodingEncoder<S,D>( withCRLF, '%' );
  }

  Decoder<S,D> * makeDecoder( bool withCRLF=false ) const {
    return new QuotedPrintableDecoder<S,D>( withCRLF, true, '%' );
  }
};

//===============================================
//
// D E F I N I T I O N S
//
//===============================================


#if defined(KMIME_CODEC_USE_CODEMAPS)
static const uchar base64DecodeMap[128] = {
  64, 64, 64, 64, 64, 64, 64, 64,  64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64,  64, 64, 64, 64, 64, 64, 64, 64,
  
  64, 64, 64, 64, 64, 64, 64, 64,  64, 64, 64, 62, 64, 64, 64, 63,
  52, 53, 54, 55, 56, 57, 58, 59,  60, 61, 64, 64, 64, 64, 64, 64,
  
  64, 64,  1,  2,  3,  4,  5,  6,   7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22,  23, 24, 25, 64, 64, 64, 64, 64,
  
  64, 26, 27, 28, 29, 30, 31, 32,  33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48,  49, 50, 51, 64, 64, 64, 64, 64
};

static const char base64EncodeMap[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};
#endif // KMIME_CODEC_USE_CODEMAPS


template <typename S, typename D>
bool Base64Decoder<S,D>::decode( S & scursor, const S & send,
				 D & dcursor, const D & dend )
{
  while ( dcursor != dend && scursor != send ) {
    uchar ch = *scursor++;
    uchar value;

#if defined(KMIME_CODEC_USE_CODEMAPS)
    // try converting ch to a 6-bit value:
    if ( ch < 128 )
      value = base64DecodeMap[ ch ];
    else
      value = 64;

    // ch isn't of the base64 alphabet, check for other significant chars:
    if ( value >= 64 ) {
      if ( ch == '=' ) {
	// padding:
	if ( mStepNo == 0 || mStepNo == 1) {
	  if (!mSawPadding) {
	    // malformed
	    kdWarning() << "Base64Decoder: unexpected padding "
	      "character in input stream" << endl;
	  }
	  mSawPadding = true;
	  break;
	} else if ( mStepNo == 2 ) {
	  // ok, there should be another one
	} else if ( mStepNo == 3 ) {
	  // ok, end of encoded stream
	  mSawPadding = true;
	  break;
	}
	mSawPadding = true;
	mStepNo++; // by definition calculated mod 4 !
	continue;
      } else {
	// non-base64 alphabet
	continue;
      }
    }
#else
    // what follows is a decision tree that aims to be efficient for
    // the (most common) case of correct base64-encoding. It looks like:
    // 0 1 2 3 4 5 (comparisons)
    // + [\0-\0xff]
    //   + [\0-Z]
    //     + [A-Z] -> OK, gives 0..25
    //     + [\0-@]
    //       + [0-@]
    //         + [0-9] -> OK, gives 52..61
    //         + [:-@]
    //           + [=] -> OK, padding
    //           + [:;<>?@] -> NO
    //       + [\0-/]
    //         + [+] -> OK, gives 62
    //         + [\0-*,-/]
    //           + [/] -> OK, gives 63
    //           + [\0-*,-.] -> NO
    //   + [[-\0xff]
    //     + [[-z]
    //       + [a-z] -> OK, gives 26..51
    //       + [[-`] -> NO
    //     + [{-\0xff] -> NO
    // => ( 26 * 2 + 26 * 3 + 10 * 4 + 4 + 5 ) / 64 = 2.8 compar./char
    if ( ch <= 'Z' ) {
      if ( ch >= 'A' ) {
	// A..Z
	value = ch - 'A';
      } else if ( ch >= '0' ) {
	if ( ch <= '9' ) {
	  // 0..9
	  value = ch - '0' + 52;
	} else if ( ch == '=' ) {
	  // padding
	  if ( mStepNo == 0 || mStepNo == 1) {
	    if (!mSawPadding) {
	      // malformed
	      kdWarning() << "Base64Decoder: unexpected padding "
		"character in input stream" << endl;
	    }
	    mSawPadding = true;
	    return true;
	  } else if ( mStepNo == 2 ) {
	    // ok, there should be another one
	  } else if ( mStepNo == 3 ) {
	    // ok, end of encoded stream
	    mSawPadding = true;
	    return true;
	  }
	  mSawPadding = true;
	  mStepNo++; // by definition calculated mod 4 !
	  continue;
	} else {
	  // non-base64 alphabet.
	  continue;
	}
      } else if ( ch == '+' ) {
	value = 62;
      } else if ( ch == '/' ) {
	value = 63;
      } else {
	// non-base64 alphabet.
	continue;
      }
    } else if ( ch <= 'z' && ch >= 'a' ) {
      // a..z
      value = ch - 'a' + 26;
    } else {
      // non-base64 alphabet.
      continue;
    }
#endif // KMIME_CODEC_USE_CODEMAPS

    if ( mSawPadding ) {
      kdWarning() << "Base64Decoder: Embedded padding character "
	"encountered!" << endl;
      return true;
    }
    
    switch ( mStepNo ) {
    case 0:
      mOutbits = value << 2;
      break;
    case 1:
      *dcursor++ = (char)(mOutbits | value >> 4);
      mOutbits = value << 4;
      break;
    case 2:
      *dcursor++ = (char)(mOutbits | value >> 2);
      mOutbits = value << 6;
      break;
    case 3:
      *dcursor++ = (char)(mOutbits | value);
      mOutbits = 0;
      break;
    default:
      assert( 0 );
    }
    mStepNo++; // by definition calculated mod 4 !
  }

  // return false when caller should call us again:
  return !(scursor != send);
} // Base64Decoder<S,D>::decode()



template <typename S, typename D>
bool Base64Encoder<S,D>::encode( S & scursor, const S & send,
				 D & dcursor, const D & dend ) {
  const uint maxPacketsPerLine = 76 / 4;

  // detect when the caller doesn't adhere to our rules:
  assert( !mInsideFinishing );

  while ( scursor != send && dcursor != dend ) {
    uchar ch;
    uchar value; // value of the current sextet
    // mNextbits   // (part of) value of next sextet

    // check for line length;
    if ( mStepNo == 0 && mWrittenPacketsOnThisLine >= maxPacketsPerLine ) {
      if ( mWithCRLF ) {
	*dcursor++ = '\r';
	// ### FIXME: when this return is taken, don't we then write
	// _two_ \r's?
	if ( !(dcursor != dend) )
	  return false;
      }
      *dcursor++ = '\n';
      mWrittenPacketsOnThisLine = 0;
      continue;
    }

    // depending on mStepNo, extract value and mNextbits from the
    // octet stream:
    switch ( mStepNo ) {
    case 0:
      assert( mNextbits == 0 );
      ch = *scursor++;
      value = ch >> 2; // top-most 6 bits -> value
      mNextbits = (ch & 0x3) << 4; // 0..1 bits -> 4..5 in mNextbits
      break;
    case 1:
      assert( (mNextbits & ~0x30) == 0 );
      ch = *scursor++;
      value = mNextbits | ch >> 4; // 4..7 bits -> 0..3 in value
      mNextbits = (ch & 0xf) << 2; // 0..3 bits -> 2..5 in mNextbits
      break;
    case 2:
      assert( (mNextbits & ~0x3C) == 0 );
      ch = *scursor++;
      value = mNextbits | ch >> 6; // 6..7 bits -> 0..1 in value
      mNextbits = ch & 0x3F;       // 0..6 bits -> mNextbits
      break;
    case 3:
      // this case is needed in order to not output more than one
      // character per round; we could write past dend else!
      assert( (mNextbits & ~0x3F) == 0 );
      value = mNextbits;
      mNextbits = 0;
      mWrittenPacketsOnThisLine++;
      break;
    default:
      value = 0; // prevent compiler warning
      assert( 0 );
    }
    mStepNo++; // by definition calculated mod 4 !

    assert( value < 64 );

    // now map the value to the corresponding base64 character:
#if defined(KMIME_CODEC_USE_CODEMAPS)
    *dcursor++ = base64EncodeMap[ value ];
#else
    // ( 26 * 1 + 26 * 2 + 10 * 3 + 2 * 4 ) / 64 = 1.8125 comp./char
    if ( value < 26 )
      value += 'A';
    else if ( value < 52 )
      value += 'a' - 26;
    else if ( value < 62 )
      value += '0' - 52;
    else if ( value < 63 )
      value = '+';
    else
      value = '/';
    *dcursor++ = char(value);
#endif
  }
  
  return !(scursor != send);
}

template <typename S, typename D>
bool Base64Encoder<S,D>::finish( D & dcursor, const D & dend ) {
  return generic_finish( dcursor, dend, true );
}

template <typename S, typename D>
bool Rfc2047BEncodingEncoder<S,D>::finish( D & dcursor, const D & dend ) {
  return generic_finish( dcursor, dend, false );
}

template <typename S, typename D>
bool Base64Encoder<S,D>::generic_finish( D & dcursor, const D & dend,
					 bool withLFatEnd ) {

  if ( !mInsideFinishing ) {

    //
    // writing out the last mNextbits...
    //
    kdDebug() << "mInsideFinishing with mStepNo == " << mStepNo << endl;
    switch ( mStepNo ) {
    case 0: // no mNextbits waiting to be written.
      assert( mNextbits == 0 );
      if ( withLFatEnd ) {
	if ( !(dcursor != dend) ) {
	  *dcursor++ = '\n';
	  return true;
	} else
	  return false;
      } else
	return true;

    case 1: // 2 mNextbits waiting to be written.
      assert( (mNextbits & ~0x30) == 0 );
      break;
      
    case 2: // 4 mNextbits waiting to be written.
      assert( (mNextbits & ~0x3C) == 0 );
      break;
      
    case 3: // 6 mNextbits waiting to be written.
      assert( (mNextbits & ~0x3F) == 0 );
      break;
    default:
      assert( 0 );
    }

    // abort write if buffer full:    
    if ( !(dcursor != dend) ) // don't force operator== on iterators
      return false;

#if defined(KMIME_CODEC_USE_CODEMAPS)
    *dcursor++ = base64EncodeMap[ mNextbits ];
#else
    if ( mNextbits < 26 )
      mNextbits += 'A';
    else if ( mNextbits < 52 )
      mNextbits += 'a' - 26;
    else if ( mNextbits < 62 )
      mNextbits += '0' - 52;
    else if ( mNextbits < 63 )
      mNextbits = '+';
    else
      mNextbits = '/';
    *dcursor++ = char(mNextbits);
#endif
    mNextbits = 0;
    
    mStepNo++; // by definition calculated mod 4 !
    mInsideFinishing = true;
  }

  //
  // adding padding...
  //
  while ( dcursor != dend ) {
    switch ( mStepNo ) {
    case 0:
      *dcursor++ = '\n';
      return true; // finished
      
    case 1:
    default:
      assert( 0 );
      break;

    case 2:
    case 3:
      *dcursor++ = '=';
      break;
    }
    mStepNo++; // by definition calculated mod 4 !
  }

  return false; // output buffer full
}





template <typename S, typename D>
bool QuotedPrintableDecoder<S,D>::decode( S & scursor, const S & send,
					  D & dcursor, const D & dend ) {
  if ( mWithCRLF )
    kdWarning() << "CRLF output for decoders isn't yet supported!" << endl;

  while ( scursor != send && dcursor != dend ) {
    if ( mFlushing ) {
      // we have to flush chars in the aftermath of an decoding
      // error. The way to request a flush is to
      // - store the offending character in mBadChar and
      // - set mFlushing to true.
      // The supported cases are (H: hexchar, X: bad char):
      // =X, =HX, CR
      // mBadChar is only written out if it is not by itself illegal in
      // quoted-printable (e.g. CTLs, 8Bits).
      // A fast way to suppress mBadChar output is to set it to NUL.
      if ( mInsideHexChar ) {
	// output '='
	*dcursor++ = mEscapeChar;
	mInsideHexChar = false;
      } else if ( mHaveAccu ) {
	// output the high nibble of the accumulator:
	uchar value = mAccu >> 4;
	if ( value > 9 )
	  value += 7;
	*dcursor++ = char( value + '0' );
	mHaveAccu = false;
	mAccu = 0;
      } else {
	// output mBadChar
	assert( mAccu == 0 );
	if ( mBadChar ) {
	  if ( mBadChar >= '>' && mBadChar <= '~' ||
	       mBadChar >= '!' && mBadChar <= '<' )
	    *dcursor++ = mBadChar;
	  mBadChar = 0;
	}
	mFlushing = false;
      }
      continue;
    }
    assert( mBadChar == 0 );

    uchar ch = *scursor++;
    uchar value = 255;

    if ( mExpectLF && ch != '\n' ) {
      kdWarning() << "QuotedPrintableDecoder: "
	"illegally formed soft linebreak or lonely CR!" << endl;
      mInsideHexChar = false;
      mExpectLF = false;
      assert( mAccu == 0 );
    }

    if ( mInsideHexChar ) {
      // next char(s) represent nibble instead of itself:
      if ( ch <= '9' ) {
	if ( ch >= '0' ) {
	  value = ch - '0';
	} else {
	  switch ( ch ) {
	  case '\r':
	    mExpectLF = true;
	    break;
	  case '\n':
	    // soft line break, but only if mAccu is NUL.
	    if ( !mHaveAccu ) {
	      mExpectLF = false;
	      mInsideHexChar = false;
	      break;
	    }
	    // else fall through
	  default:
	    kdWarning() << "QuotedPrintableDecoder: "
	      "illegally formed hex char! Outputting verbatim." << endl;
	    mBadChar = ch;
	    mFlushing = true;
	  }
	  continue;
	}
      } else { // ch > '9'
	if ( ch <= 'F' ) {
	  if ( ch >= 'A' ) {
	    value = 10 + ch - 'A';
	  } else { // [:-@]
	    mBadChar = ch;
	    mFlushing = true;
	    continue;
	  }
	} else { // ch > 'F'
	  if ( ch <= 'f' && ch >= 'a' ) {
	    value = 10 + ch - 'a';
	  } else {
	    mBadChar = ch;
	    mFlushing = true;
	    continue;
	  }
	}
      }

      assert( value < 16 );
      assert( mBadChar == 0 );
      assert( !mExpectLF );

      if ( mHaveAccu ) {
	*dcursor++ = char( mAccu | value );
	mAccu = 0;
	mHaveAccu = false;
	mInsideHexChar = false;
      } else {
	mHaveAccu = true;
	mAccu = value << 4;
      }
    } else { // not mInsideHexChar
      if ( ch <= '~' && ch >= ' ' || ch == '\t' ) {
	if ( ch == mEscapeChar ) {
	  mInsideHexChar = true;
	} else if ( mQEncoding && ch == '_' ) {
	  *dcursor++ = char(0x20);
	} else {
	  *dcursor++ = char(ch);
	}
      } else if ( ch == '\n' ) {
	*dcursor++ = '\n';
	mExpectLF = false;
      } else if ( ch == '\r' ) {
	mExpectLF = true;
      } else {
	kdWarning() << "QuotedPrintableDecoder: " << ch <<
	  " illegal character in input stream! Ignoring." << endl;
      }
    }
  }

  return !(scursor != send);
}

template <typename S, typename D>
bool QuotedPrintableEncoder<S,D>::fillInputBuffer( S & scursor,
						   const S & send ) {
  // Don't read more if there's still a tail of a line in the buffer:
  if ( mSawLineEnd )
    return true;

  // Read until the buffer is full or we have found CRLF or LF (which
  // don't end up in the input buffer):
  for ( ; ( mInputBufferWriteCursor + 1 ) % 16 != mInputBufferReadCursor
	  && scursor != send ; mInputBufferWriteCursor++ ) {
    char ch = *scursor++;
    if ( ch == '\r' ) {
      mSawCR = true;
    } else if ( ch == '\n' ) {
      // remove the CR from the input buffer (if any) and return that
      // we found a line ending:
      if ( mSawCR ) {
	mSawCR = false;
	assert( mInputBufferWriteCursor != mInputBufferReadCursor );
	mInputBufferWriteCursor--;
      }
      mSawLineEnd = true;
      return true; // saw CRLF or LF
    } else {
      mSawCR = false;
    }
    mInputBuffer[ mInputBufferWriteCursor ] = ch;
  }
  mSawLineEnd = false;
  return false; // didn't see a line ending...
}

template <typename S, typename D>
bool QuotedPrintableEncoder<S,D>::processNextChar() {

  // If we process a buffer which doesn't end in a line break, we
  // can't process all of it, since the next chars that will be read
  // could be a line break. So we empty the buffer only until a fixed
  // number of chars is left:
  const int minBufferFillWithoutLineEnd = 4;

  assert( mOutputBufferCursor == 0 );

  int bufferFill = int(mInputBufferWriteCursor) - int(mInputBufferReadCursor) ;
  if ( bufferFill < 0 )
    bufferFill += 16;

  assert( bufferFill >=0 && bufferFill <= 15 );
  
  if ( !mSawLineEnd && bufferFill < minBufferFillWithoutLineEnd )
    return false;

  // buffer is empty, return false:
  if ( mInputBufferReadCursor == mInputBufferWriteCursor )
    return false;

  // Real processing goes here:
  mAccu = mInputBuffer[ mInputBufferReadCursor++ ];
  if ( needsEncoding( mAccu ) || // always needs encoding or
       mSawLineEnd && bufferFill == 1 // needs encoding at end of line
       && ( mAccu == ' ' || mAccu == '\t' ) )
    mAccuNeedsEncoding = Definitely;
  else if ( mAccu == '-' || mAccu == 'F' || mAccu == '.' )
    // needs encoding at beginning of line
    mAccuNeedsEncoding = AtBOL;
  else
    // never needs encoding
    mAccuNeedsEncoding = Never;
  
  return true;
}

// Outputs processed (verbatim or hex-encoded) chars and inserts soft
// line breaks as necessary. Depends on processNextChar's directions
// on whether or not to encode the current char, and whether or not
// the current char is the last one in it's input line:
template <typename S, typename D>
void QuotedPrintableEncoder<S,D>::createOutputBuffer() {
  const int maxLineLength = 76; // rfc 2045

  assert( mOutputBufferCursor == 0 );

  bool lastOneOnThisLine = mSawLineEnd
    && mInputBufferReadCursor == mInputBufferWriteCursor;

  int neededSpace = 1;
  if ( mAccuNeedsEncoding == Definitely)
    neededSpace = 3;

  // reserve space for the soft hyphen (=)
  if ( !lastOneOnThisLine )
    neededSpace++;

  if ( mCurrentLineLength > maxLineLength - neededSpace ) {
      // current line too short, insert soft line break:
      mOutputBuffer[ mOutputBufferCursor++ ] = '=';
      if ( mWithCRLF )
	mOutputBuffer[ mOutputBufferCursor++ ] = '\r';
      mOutputBuffer[ mOutputBufferCursor++ ] = '\n';
      mCurrentLineLength = 0;
  }

  if ( Never == mAccuNeedsEncoding ||
       AtBOL == mAccuNeedsEncoding && mCurrentLineLength ) {
    mOutputBuffer[ mOutputBufferCursor++ ] = mAccu;
    mCurrentLineLength++;
  } else {
    mOutputBuffer[ mOutputBufferCursor++ ] = '=';
    uchar value;
    value = mAccu >> 4;
    if ( value > 9 )
      value += 'A' - 10;
    else
      value += '0';
    mOutputBuffer[ mOutputBufferCursor++ ] = char(value);
    value = mAccu & 0xF;
    if ( value > 9 )
      value += 'A' - 10;
    else
      value += '0';
    mOutputBuffer[ mOutputBufferCursor++ ] = char(value);
    mCurrentLineLength += 3;
  }
}

template <typename S, typename D>
bool QuotedPrintableEncoder<S,D>::encode( S & scursor, const S & send,
					  D & dcursor, const D & dend ) {
  assert ( !mFinishing );

  uint i = 0;

  while ( scursor != send && dcursor != dend ) {
    if ( mOutputBufferCursor ) {
      assert( mOutputBufferCursor <= 6 );
      if ( i < mOutputBufferCursor ) {
	*dcursor++ = mOutputBuffer[i++];
	continue;
      } else {
	mOutputBufferCursor = i = 0;
      }
    }

    assert( mOutputBufferCursor == 0 );

    fillInputBuffer( scursor, send );
    
    if ( processNextChar() )
      // there was one...
      createOutputBuffer();
    else if ( mSawLineEnd &&
	      mInputBufferWriteCursor == mInputBufferReadCursor ) {
      // load a hard line break into output buffer:
      if ( mWithCRLF )
	mOutputBuffer[ mOutputBufferCursor++ ] = '\r';
      mOutputBuffer[ mOutputBufferCursor++ ] = '\n';
      mSawLineEnd = false;
      mCurrentLineLength = 0;
    } else 
      break;
  }

  if ( i && i < mOutputBufferCursor ) {
    // adjust output buffer:
    std::memmove( mOutputBuffer, &mOutputBuffer[i], mOutputBufferCursor - i );
    mOutputBufferCursor -= i;
  } else if ( i == mOutputBufferCursor ) {
    mOutputBufferCursor = i = 0;
  }

  return !(scursor != send);
   
} // encode

template <typename S, typename D>
bool QuotedPrintableEncoder<S,D>::finish( D & dcursor, const D & dend ) {
  mFinishing = true;

  uint i = 0;

  while ( dcursor != dend ) {
    // empty output buffer:
    if ( mOutputBufferCursor ) {
      assert( mOutputBufferCursor <= 6 );
      if ( i < mOutputBufferCursor ) {
	*dcursor++ = mOutputBuffer[i++];
	continue;
      } else {
	mOutputBufferCursor = i = 0;
      }
    }

    assert( mOutputBufferCursor == 0 );

    if ( processNextChar() )
      // there was one...
      createOutputBuffer();
    else if ( mSawLineEnd &&
	      mInputBufferWriteCursor == mInputBufferReadCursor ) {
      // load a hard line break into output buffer:
      if ( mWithCRLF )
	mOutputBuffer[ mOutputBufferCursor++ ] = '\r';
      mOutputBuffer[ mOutputBufferCursor++ ] = '\n';
      mSawLineEnd = false;
      mCurrentLineLength = 0;
    } else 
      break;
  }

  if ( i && i < mOutputBufferCursor ) {
    // adjust output buffer:
    memmove( mOutputBuffer, &mOutputBuffer[i], mOutputBufferCursor - i );
    mOutputBufferCursor -= i;
  } else if ( i == mOutputBufferCursor ) {
    mOutputBufferCursor = i = 0;
  }

  // ### Most probably not correct...
  return !mOutputBufferCursor
    && mInputBufferReadCursor == mInputBufferWriteCursor;
} // finish


template <typename S, typename D>
bool Rfc2047QEncodingEncoder<S,D>::encode( S & scursor, const S & send,
					   D & dcursor, const D & dend )
{
  while ( scursor != send && dcursor != dend ) {
    uchar value;
    switch ( mStepNo ) {
    case 0:
      mAccu = *scursor++;
      // <= 'z' is an optimization, since we know that {|}~ are not
      // etext:
      if ( mAccu <= 'z' && isEText( mAccu )
	   && mAccu != (unsigned char)mEscapeChar ) {
	*dcursor++ = char(mAccu);
      } else if ( mEscapeChar == '=' && mAccu == 0x20 ) {
	// shortcut encoding for 0x20 (latin-1/us-ascii SPACE)
	// (not for rfc2231 encoding)
	*dcursor++ = '_';
      } else {
	*dcursor++ = mEscapeChar;
	mStepNo = 1;
      }
      continue;
    case 1:
      value = mAccu >> 4;
      mStepNo = 2;
      break;
    case 2:
      value = mAccu & 0xF;
      mStepNo = 0;
      break;
    default: assert( 0 );
    }

    if ( value > 9 )
      value += 'A' - 10;
    else
      value += '0';
    *dcursor++ = char(value);
  }

  return !(scursor != send);
} // encode

template <typename S, typename D>
bool Rfc2047QEncodingEncoder<S,D>::finish( D & dcursor, const D & dend ) {

  while ( !mStepNo && dcursor != dend ) {
    uchar value;
    switch ( mStepNo ) {
    case 1:
      value = mAccu >> 4;
      mStepNo = 2;
      break;
    case 2:
      value = mAccu & 0xF;
      mStepNo = 0;
      break;
    default: assert( 0 );
    }

    if ( value > 9 )
      value += 'A' - 10;
    else
      value += '0';
    *dcursor++ = char(value);
  }

  return mStepNo == 0;
};

static inline bool keep( uchar ch ) {
  // no CTLs, except HT and not '?'
  return !( ch < ' ' && ch != '\t' || ch == '?' );
}

}; // namespace KMime

// explicit instantation of the most often used instance:
template KMime::Codec<char*,char*>;
