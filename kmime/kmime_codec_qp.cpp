/*
    kmime_codec_qp.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2002 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, US
*/

#include "kmime_codec_qp.h"

#include "kmime_util.h"

#include <kdebug.h>

#include <cassert>

using namespace KMime;

namespace KMime {


//
// QuotedPrintableCodec
//

class QuotedPrintableEncoder : public Encoder {
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
  friend class QuotedPrintableCodec;
  QuotedPrintableEncoder( bool withCRLF=false )
    : Encoder(), mCurrentLineLength(0), mAccu(0),
      mInputBufferReadCursor(0), mInputBufferWriteCursor(0),
      mOutputBufferCursor(0), mAccuNeedsEncoding(Never),
      mSawLineEnd(false), mSawCR(false), mFinishing(false),
      mWithCRLF( withCRLF ) {}

  bool needsEncoding( uchar ch ) {
    return ( ch > '~' || ch < ' ' && ch != '\t' || ch == '=' );
  }
  bool fillInputBuffer( const char* & scursor, const char * const send );
  bool processNextChar();
  void createOutputBuffer();
public:
  virtual ~QuotedPrintableEncoder() {}

  bool encode( const char* & scursor, const char * const send,
	       char* & dcursor, const char * const dend );

  bool finish( char* & dcursor, const char * const dend );
};


class QuotedPrintableDecoder : public Decoder {
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
  friend class QuotedPrintableCodec;
  friend class Rfc2047QEncodingCodec;
  friend class Rfc2231EncodingCodec;
  QuotedPrintableDecoder( bool withCRLF=false,
			  bool aQEncoding=false, char aEscapeChar='=' )
    : Decoder(),
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

  bool decode( const char* & scursor, const char * const send,
	       char* & dcursor, const char * const dend );
  // ### really no finishing needed???
  bool finish( char* &, const char * const ) { return true; }
};


class Rfc2047QEncodingEncoder : public Encoder {
  uchar      mAccu;
  uchar      mStepNo;
  const char mEscapeChar;
  const bool mWithCRLF : 1;
  bool       mInsideFinishing : 1;
protected:
  friend class Rfc2047QEncodingCodec;
  friend class Rfc2231EncodingCodec;
  Rfc2047QEncodingEncoder( bool withCRLF=false, char aEscapeChar='=' )
    : Encoder(),
      mAccu(0), mStepNo(0), mEscapeChar( aEscapeChar ),
      mWithCRLF( withCRLF ), mInsideFinishing( false )
  {
    // else an optimization in ::encode might break.
    assert( aEscapeChar == '=' || aEscapeChar == '%' );
  }

  // this code assumes that isEText( mEscapeChar ) == false!
  bool needsEncoding( uchar ch ) {
    if ( ch > 'z' ) return true; // {|}~ DEL and 8bit chars need
    if ( !isEText( ch ) ) return true; // all but a-zA-Z0-9!/*+- need, too
    if ( mEscapeChar == '%' && ( ch == '*' || ch == '/' ) )
      return true; // not allowed in rfc2231 encoding
    return false;
  }

public:
  virtual ~Rfc2047QEncodingEncoder() {}

  bool encode( const char* & scursor, const char * const send,
	       char* & dcursor, const char * const dend );
  bool finish( char* & dcursor, const char * const dend );
};


Encoder * QuotedPrintableCodec::makeEncoder( bool withCRLF ) const {
  return new QuotedPrintableEncoder( withCRLF );
}

Decoder * QuotedPrintableCodec::makeDecoder( bool withCRLF ) const {
  return new QuotedPrintableDecoder( withCRLF );
}

Encoder * Rfc2047QEncodingCodec::makeEncoder( bool withCRLF ) const {
  return new Rfc2047QEncodingEncoder( withCRLF );
}

Decoder * Rfc2047QEncodingCodec::makeDecoder( bool withCRLF ) const {
  return new QuotedPrintableDecoder( withCRLF, true );
}

Encoder * Rfc2231EncodingCodec::makeEncoder( bool withCRLF ) const {
  return new Rfc2047QEncodingEncoder( withCRLF, '%' );
}

Decoder * Rfc2231EncodingCodec::makeDecoder( bool withCRLF ) const {
  return new QuotedPrintableDecoder( withCRLF, true, '%' );
}


  /********************************************************/
  /********************************************************/
  /********************************************************/


bool QuotedPrintableDecoder::decode( const char* & scursor, const char * const send,
				     char* & dcursor, const char * const dend ) {
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

  return (scursor == send);
}

bool QuotedPrintableEncoder::fillInputBuffer( const char* & scursor,
					      const char * const send ) {
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

bool QuotedPrintableEncoder::processNextChar() {

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
void QuotedPrintableEncoder::createOutputBuffer() {
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

bool QuotedPrintableEncoder::encode( const char* & scursor, const char * const send,
				     char* & dcursor, const char * const dend ) {
  if ( mFinishing ) return true;

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
    memmove( mOutputBuffer, &mOutputBuffer[i], mOutputBufferCursor - i );
    mOutputBufferCursor -= i;
  } else if ( i == mOutputBufferCursor ) {
    mOutputBufferCursor = i = 0;
  }

  return (scursor == send);
   
} // encode

bool QuotedPrintableEncoder::finish( char* & dcursor,
				     const char * const dend ) {
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


bool Rfc2047QEncodingEncoder::encode( const char* & scursor, const char * const send,
				      char* & dcursor, const char * const dend )
{
  if ( mInsideFinishing ) return true;

  while ( scursor != send && dcursor != dend ) {
    uchar value;
    switch ( mStepNo ) {
    case 0:
      // read the next char and decide if and how do encode:
      mAccu = *scursor++;
      if ( !needsEncoding( mAccu ) ) {
	*dcursor++ = char(mAccu);
      } else if ( mEscapeChar == '=' && mAccu == 0x20 ) {
	// shortcut encoding for 0x20 (latin-1/us-ascii SPACE)
	// (not for rfc2231 encoding)
	*dcursor++ = '_';
      } else {
	// needs =XY encoding - write escape char:
	*dcursor++ = mEscapeChar;
	mStepNo = 1;
      }
      continue;
    case 1:
      // extract hi-nibble:
      value = mAccu >> 4;
      mStepNo = 2;
      break;
    case 2:
      // extract lo-nibble:
      value = mAccu & 0xF;
      mStepNo = 0;
      break;
    default: assert( 0 );
    }

    // convert value to hexdigit:
    if ( value > 9 )
      value += 'A' - 10;
    else
      value += '0';
    // and write:
    *dcursor++ = char(value);
  }

  return (scursor == send);
} // encode

#include <qstring.h>

bool Rfc2047QEncodingEncoder::finish( char* & dcursor, const char * const dend ) {
  mInsideFinishing = true;

  kdDebug() << "mInsideFinishing with mStepNo = " << mStepNo << " and mAccu = "
	    << QString(QChar(mAccu)) << endl;

  // write the last bits of mAccu, if any:
  while ( mStepNo != 0 && dcursor != dend ) {
    uchar value;
    switch ( mStepNo ) {
    case 1:
      // extract hi-nibble:
      value = mAccu >> 4;
      mStepNo = 2;
      break;
    case 2:
      // extract lo-nibble:
      value = mAccu & 0xF;
      mStepNo = 0;
      break;
    default: assert( 0 );
    }

    // convert value to hexdigit:
    if ( value > 9 )
      value += 'A' - 10;
    else
      value += '0';
    // and write:
    *dcursor++ = char(value);
  }

  return mStepNo == 0;
};

static inline bool keep( uchar ch ) {
  // no CTLs, except HT and not '?'
  return !( ch < ' ' && ch != '\t' || ch == '?' );
}




}; // namespace KMime
