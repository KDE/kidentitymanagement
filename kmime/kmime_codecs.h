/*
    kmime_codecs.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef __KMIME_CODECS__
#define __KMIME_CODECS__

#include <qasciidict.h>
#if defined(QT_THREAD_SUPPORT) && defined(KMIME_REALLY_USE_THREADS)
#  include <qmutex.h>
#endif

class QCString;

namespace KMime {

template <typename S, typename D>
class Encoder;
template <typename S, typename D>
class Decoder;

/** Abstract base class of codecs like base64 and
    quoted-printable. It's a singleton. */
template <typename S=char*, typename D=char*>
class Codec {
  static QAsciiDict< Codec<S,D> > all;
#if defined(QT_THREAD_SUPPORT) && defined(KMIME_REALLY_USE_THREADS)
  static QMutex dictLock;
#endif
protected:
  Codec() {}
private:
  static void fillDictionary();

public:
  static Codec * codecForName( const char * name );
  static Codec * codecForName( const QCString & name );

  virtual Encoder<S,D> * makeEncoder() const = 0;
  virtual Decoder<S,D> * makeDecoder() const = 0;

  virtual void encode( S & scursor, const S & send,
		       D & dcursor, const D & dend,
		       bool withCRLF=false ) const;

  virtual void decode( S & scursor, const S & send,
		       D & dcursor, const D & dend ) const;

  virtual const char * name() const = 0;

  virtual ~Codec() {}

};

/** Stateful decoder class */
template <typename S=char*, typename D=char*>
class Decoder {
protected:
  friend class Codec<S,D>;
  Decoder() {};
public:
  virtual ~Decoder() {};

  /** Decode a chunk of data, maintaining state information between
      calls. */
  virtual void decode( S & scursor, const S & send,
		       D & dcursor, const D & dend ) = 0;

  /** Call this method to check whether the decoder saw an
      end-of-encoding marker, if somthing like that exists for that
      specific encoding. */
  virtual bool sawEnd() const { return false; }
};

/** Stateful encoder class */
template <typename S=char*, typename D=char*>
class Encoder {
protected:
  friend class Codec<S,D>;
  Encoder() {};
public:
  virtual ~Encoder() {};

  /** Encode a chunk of data, maintaining state information between
      calls. */
  virtual void encode( S & scursor, const S & send,
		       D & dcursor, const D & dend, bool withCRLF=false ) = 0;

  /** Call this method to finalize the output stream. Writes all
      remaining data and resets the encoder. */
  virtual void finish( D & dcursor, const D & dend, bool withCRLF=false ) {}
};

}; // namespace KMime

#endif // __KMIME_CODECS__
