/*
    kmime_codec_base64.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, US
*/

#ifndef __KMIME_CODEC_BASE64__
#define __KMIME_CODEC_BASE64__

#include "kmime_codecs.h"

namespace KMime {

class Base64Codec : public Codec {
protected:
  friend class Codec;
  Base64Codec() : Codec() {}

public:
  virtual ~Base64Codec() {}

  const char * name() const {
    return "base64";
  }

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};



class Rfc2047BEncodingCodec : public Base64Codec {
protected:
  friend class Codec;
  Rfc2047BEncodingCodec()
    : Base64Codec() {}
  
public:
  virtual ~Rfc2047BEncodingCodec() {}

  const char * name() const { return "b"; }

  Encoder * makeEncoder( bool withCRLF=false ) const;
};


}; // namespace KMime

#endif // __KMIME_CODEC_BASE64__
