/*
    kmime_codec_base64.cpp

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

#include "kmime_codec_base64.h"

#include <kdebug.h>

#include <cassert>

using namespace KMime;

namespace KMime {

// codec for base64 as specified in RFC 2045
  //class Base64Codec;
  //class Base64Decoder;
  //class Base64Encoder;

// codec for the B encoding as specified in RFC 2047
  //class Rfc2047BEncodingCodec;
  //class Rfc2047BEncodingEncoder;
  //class Rfc2047BEncodingDecoder;



class Base64Decoder : public Decoder {
  uint mStepNo;
  uchar mOutbits;
  bool mSawPadding : 1;
  const bool mWithCRLF : 1;

protected:
  friend class Base64Codec;
  Base64Decoder( bool withCRLF=false )
    : Decoder( withCRLF ), mOutbits(0), mStepNo(0),
      mSawPadding(false), mWithCRLF( withCRLF ) {}

public:
  virtual ~Base64Decoder() {}

  bool decode( const char* & scursor, const char * const send,
	       char* & dcursor, const char * const dend );
  // ### really needs no finishing???
  bool finish( char* & /*dcursor*/, const char * const /*dend*/ ) { return true; }
};



class Base64Encoder : public Encoder {
  uint mStepNo;
  /** number of already written base64-quartets on current line */
  uint mWrittenPacketsOnThisLine;
  uchar mNextbits;
  bool mInsideFinishing : 1;
  const bool mWithCRLF : 1;

protected:
  friend class Rfc2047BEncodingCodec;
  friend class Rfc2047BEncodingEncoder;
  friend class Base64Codec;
  Base64Encoder( bool withCRLF=false )
    : Encoder(), mStepNo(0), mWrittenPacketsOnThisLine(0),
      mNextbits(0), mInsideFinishing(false),
      mWithCRLF( withCRLF ) {}

  bool generic_finish( char* & dcursor, const char * const dend,
		       bool withLFatEnd );

public:
  virtual ~Base64Encoder() {}

  bool encode( const char* & scursor, const char * const send,
	       char* & dcursor, const char * const dend );

  bool finish( char* & dcursor, const char * const dend );
};



class Rfc2047BEncodingEncoder : public Base64Encoder {
protected:
  friend class Rfc2047BEncodingCodec;
  Rfc2047BEncodingEncoder( bool withCRLF=false )
    : Base64Encoder( withCRLF ) {};
public:
  bool encode( const char* & scursor, const char * const send,
	       char* & dcursor, const char * const send );
  bool finish( char* & dcursor, const char * const dend );
};


Encoder * Base64Codec::makeEncoder( bool withCRLF=false ) const {
  return new Base64Encoder( withCRLF );
}

Decoder * Base64Codec::makeDecoder( bool withCRLF=false ) const {
  return new Base64Decoder( withCRLF );
}

Encoder * Rfc2047BEncodingCodec::makeEncoder( bool withCRLF=false ) const {
  return new Rfc2047BEncodingEncoder( withCRLF );
}

  /********************************************************/
  /********************************************************/
  /********************************************************/


static const uchar base64DecodeMap[128] = {
  64, 64, 64, 64, 64, 64, 64, 64,  64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64,  64, 64, 64, 64, 64, 64, 64, 64,
  
  64, 64, 64, 64, 64, 64, 64, 64,  64, 64, 64, 62, 64, 64, 64, 63,
  52, 53, 54, 55, 56, 57, 58, 59,  60, 61, 64, 64, 64, 64, 64, 64,
  
  64,  0,  1,  2,  3,  4,  5,  6,   7,  8,  9, 10, 11, 12, 13, 14,
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


bool Base64Decoder::decode( const char* & scursor, const char * const send,
			    char* & dcursor, const char * const dend )
{
  while ( dcursor != dend && scursor != send ) {
    uchar ch = *scursor++;
    uchar value;

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
	mStepNo = (mStepNo + 1) % 4;
	continue;
      } else {
	// non-base64 alphabet
	continue;
      }
    }

    if ( mSawPadding ) {
      kdWarning() << "Base64Decoder: Embedded padding character "
	"encountered!" << endl;
      return true;
    }

    // add the new bits to the output stream and flush full octets:    
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
    mStepNo = (mStepNo + 1) % 4;
  }

  // return false when caller should call us again:
  return (scursor == send);
} // Base64Decoder::decode()



bool Base64Encoder::encode( const char* & scursor, const char * const send,
			    char* & dcursor, const char * const dend ) {
  const uint maxPacketsPerLine = 76 / 4;

  // detect when the caller doesn't adhere to our rules:
  if( mInsideFinishing ) return true;

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
	if ( dcursor == dend )
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
    mStepNo = ( mStepNo + 1 ) % 4;

    assert( value < 64 );

    // now map the value to the corresponding base64 character:
    *dcursor++ = base64EncodeMap[ value ];
  }
  
  return (scursor == send);
}

bool Rfc2047BEncodingEncoder::encode( const char* & scursor, const char * const send,
				      char* & dcursor, const char * const dend ) {
  // detect when the caller doesn't adhere to our rules:
  if ( mInsideFinishing ) return true;

  while ( scursor != send && dcursor != dend ) {
    uchar ch;
    uchar value; // value of the current sextet
    // mNextbits   // (part of) value of next sextet

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
    mStepNo = ( mStepNo + 1 ) % 4;

    assert( value < 64 );

    // now map the value to the corresponding base64 character:
    *dcursor++ = base64EncodeMap[ value ];
  }
  
  return (scursor == send);
}

bool Base64Encoder::finish( char* & dcursor, const char * const dend ) {
  return generic_finish( dcursor, dend, true );
}

bool Rfc2047BEncodingEncoder::finish( char* & dcursor,
				      const char * const dend ) {
  return generic_finish( dcursor, dend, false );
}

bool Base64Encoder::generic_finish( char* & dcursor, const char * const dend,
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
	if ( dcursor == dend ) {
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
    if ( dcursor == dend )
      return false;

    *dcursor++ = base64EncodeMap[ mNextbits ];
    mNextbits = 0;
    
    mStepNo = ( mStepNo + 1 ) % 4;
    mInsideFinishing = true;
  }

  //
  // adding padding...
  //
  while ( dcursor != dend ) {
    switch ( mStepNo ) {
    case 0:
      if ( withLFatEnd )
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
    mStepNo = ( mStepNo + 1 ) % 4;
  }

  if ( mStepNo == 0 && !withLFatEnd )
    return true; // just fits into output buffer
  else
    return false; // output buffer full
}






}; // namespace KMime
