/*
    kmime_codecs.h

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

#ifndef __KMIME_CODECS__
#define __KMIME_CODECS__

#include <qasciidict.h>
#if defined(QT_THREAD_SUPPORT)
#  include <qmutex.h>
#endif

class QCString;

namespace KMime {

// forward declarations:
template <typename S, typename D>
class Encoder;
template <typename S, typename D>
class Decoder;

/** Abstract base class of codecs like base64 and
    quoted-printable. It's a singleton.
    
    @short Codecs for common mail transfer encodings.
    @author Marc Mutz <mutz@kde.org>
*/
template <typename S=char*, typename D=char*>
class Codec {
protected:

  static QAsciiDict< Codec<S,D> > all;
#if defined(QT_THREAD_SUPPORT)
  static QMutex dictLock;
#endif

  Codec() {}
private:
  static void fillDictionary();
  
public:
  static Codec * codecForName( const char * name );
  static Codec * codecForName( const QCString & name );
  
  virtual Encoder<S,D> * makeEncoder( bool withCRLF=false ) const = 0;
  virtual Decoder<S,D> * makeDecoder( bool withCRLF=false ) const = 0;
  
  virtual bool encode( S & scursor, const S & send,
		       D & dcursor, const D & dend,
		       bool withCRLF=false ) const;
  
  virtual bool decode( S & scursor, const S & send,
		       D & dcursor, const D & dend,
		       bool withCRLF=false ) const;
  
  virtual const char * name() const = 0;
  
  virtual ~Codec() {}
  
};
  
/** Stateful decoder class, modelled after @ref QTextDecoder.
    @short Stateful decoder class
    @author Marc Mutz <mutz@kde.org>
*/
template <typename S=char*, typename D=char*>
class Decoder {
protected:
  friend class Codec<S,D>;
  /** Protected constructor. Use @ref KMime::Codec::makeDecoder if you
      want one. The bool parameter determines whether lines end with
      CRLF (true) or LF (false, default). */
  Decoder( bool=false ) {};
public:
  virtual ~Decoder() {};

  /** Decode a chunk of data, maintaining state information between
      calls. See @ref KMime::Codec for calling conventions. */
  virtual bool decode( S & scursor, const S & send,
		       D & dcursor, const D & dend ) = 0;
  /** Call this method to finalize the output stream. Writes all
      remaining data and resets the decoder. See @ref KMime::Codec for
      calling conventions. */
  virtual bool finish( D & dcursor, const D & dend ) = 0;
};
  
/** Stateful encoder class, modelled after @ref QTextEncoder.
    @short Stateful encoder class
    @author Marc Mutz <mutz@kde.org>
*/
template <typename S=char*, typename D=char*>
class Encoder {
protected:
  friend class Codec<S,D>;
  Encoder( bool=false ) {};
public:
  virtual ~Encoder() {};

  /** Encode a chunk of data, maintaining state information between
      calls. See @ref KMime::Codec for calling conventions. */
  virtual bool encode( S & scursor, const S & send,
		       D & dcursor, const D & dend ) = 0;

  /** Call this method to finalize the output stream. Writes all
      remaining data and resets the encoder. See @ref KMime::Codec for
      calling conventions. */
  virtual bool finish( D & dcursor, const D & dend ) = 0;
};

}; // namespace KMime

#endif // __KMIME_CODECS__
