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
    quoted-printable. It's a singleton. Use them like you'd use a @ref
    QTextCodec.

    @short Base class for Content-Transfer-Encodings
    @author Marc Mutz <mutz@kde.org>
*/
template <typename S=char*, typename D=char*>
class Codec {
  static QAsciiDict< Codec<S,D> > all;
#if defined(QT_THREAD_SUPPORT) && defined(KMIME_REALLY_USE_THREADS)
  static QMutex dictLock;
#endif
protected:
  // can only be produced via codecForName()!
  Codec() {}
private:
  static void fillDictionary();

public:
  /** Get a codec by name. Only method to create one. Don't delete it!  */
  static Codec<S,D> * codecForName( const char * name );
  /** Get a codec by name. Only method to create one. Don't delete it!  */
  static Codec<S,D> * codecForName( const QCString & name );

  /** Create a fresh encoder */
  virtual Encoder<S,D> * makeEncoder() const = 0;
  /** Create a fresh decoder */
  virtual Decoder<S,D> * makeDecoder() const = 0;

  /** Convenience function. It's your responsibility to ensure that
      the output buffer is large enough. */
  virtual void encode( S & scursor, const S & send,
		       D & dcursor, const D & dend,
		       bool withCRLF=false ) const;

  /** Convenience function. It's your responsibility to ensure that
      the output buffer is large enough. */
  virtual void decode( S & scursor, const S & send,
		       D & dcursor, const D & dend,
		       bool withCRLF=false ) const;

  /** Return the preferred name of the encoding */
  virtual const char * name() const = 0;

  virtual ~Codec() {}

};

/** Stateful decoder class */
template <typename S=char*, typename D=char*>
class Decoder {
protected:
  // can only be produced by the corresponding Codec!
  friend class Codec<S,D>;
  Decoder() {};
public:
  virtual ~Decoder() {};

  /** Decode a chunk of data, maintaining state information between
      calls. Call with @p scursor == @p send to flush the output,
      which may be still be buffered inside. Typical usage to process
      a contiguous block of input:
      <pre>
      XYZ::Iterator sit = input.begin();
      XYZ::Iterator send = input.end();
      ABC::Iterator dit, dend;
      do {
        dit = output.begin();
	dend = output.end();
        dec->decode( sit, send, dit, dend );
	doSomethingWith( output );
      } while ( sit != send || dit != output.begin() );
      </pre>
      It is important that you not only test whether the decoder has
      consumed all available input, but also that it hasn't output
      anything anymore!

      You musn't wait for @p dit == @p output.begin() if you have a
      new input chunk ready. Only after the @em last input chunk is it
      necessary and permitted to test @p dit, too.
      
      Note also, that it is perfectly permissible for the decoder to
      only read or only write during a call to decode, though it will
      always do at least one read or write. */
  virtual void decode( S & scursor, const S & send,
		       D & dcursor, const D & dend, bool withCRLF=false ) = 0;
};

/** Stateful encoder class */
template <typename S=char*, typename D=char*>
class Encoder {
protected:
  // can only be produced by the corresponding Codec!
  friend class Codec<S,D>;
  Encoder() {};
public:
  virtual ~Encoder() {};

  /** Encode a chunk of data, maintaining state information between
      calls. See @ref Encoder::encode for (important!) instructions on
      use.
  */
  virtual void encode( S & scursor, const S & send,
		       D & dcursor, const D & dend, bool withCRLF=false ) = 0;
};

}; // namespace KMime

#endif // __KMIME_CODECS__
