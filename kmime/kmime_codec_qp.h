/*  -*- c++ -*-
    kmime_codec_qp.h

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

#ifndef __KMIME_CODEC_QP__
#define __KMIME_CODEC_QP__

#include "kmime_codecs.h"

namespace KMime {


class QuotedPrintableCodec : public Codec {
protected:
  friend class Codec;
  QuotedPrintableCodec() : Codec() {}

public:
  virtual ~QuotedPrintableCodec() {}

  const char * name() const {
    return "quoted-printable";
  }

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};


class Rfc2047QEncodingCodec : public Codec {
protected:
  friend class Codec;
  Rfc2047QEncodingCodec() : Codec() {}

public:
  virtual ~Rfc2047QEncodingCodec() {}

  const char * name() const {
    return "q";
  }

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};


class Rfc2231EncodingCodec : public Codec {
protected:
  friend class Codec;
  Rfc2231EncodingCodec() : Codec() {}

public:
  virtual ~Rfc2231EncodingCodec() {}

  const char * name() const {
    return "x-kmime-rfc2231";
  }

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};


}; // namespace KMime

#endif // __KMIME_CODEC_QP__
