/*
    kmime_headers.cpp

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


#include "kmime_headers.h"

#include "kmime_util.h"
#include "kmime_content.h"
#include "kmime_codecs.h"

#include "kqcstringsplitter.h"

#include <qtextcodec.h>
#include <qstring.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <kglobal.h>
#include <kcharsets.h>
#include <krfcdate.h>

#include <assert.h>

#ifndef KMIME_NO_WARNING
#  include <kdebug.h>
#  define KMIME_WARN kdWarning(5100) << "Tokenizer Warning: "
#  define KMIME_WARN_UNKNOWN(x,y) KMIME_WARN << "unknown " #x ": \"" \
          << y << "\"" << endl;
#  define KMIME_WARN_UNKNOWN_ENCODING KMIME_WARN << "unknown encoding in " \
          "RFC 2047 encoded-word (only know 'q' and 'b')" << endl;
#  define KMIME_WARN_UNKNOWN_CHARSET(c) KMIME_WARN << "unknown charset \"" \
          << c << "\" in RFC 2047 encoded-word" << endl;
#  define KMIME_WARN_8BIT(ch) KMIME_WARN \
          << "8Bit character '" << QString(QChar(ch)) << "' in header \"" \
          << type() << '\"' << endl
#  define KMIME_WARN_IF_8BIT(ch) if ( (unsigned char)(ch) > 127 ) \
          { KMIME_WARN_8BIT(ch); }
#  define KMIME_WARN_PREMATURE_END_OF(x) KMIME_WARN \
          << "Premature end of " #x " in header \"" << type() << '\"' << endl
#  define KMIME_WARN_LONE(x) KMIME_WARN << "Lonely " #x " character in" \
          " header \"" << type() << '\"' << endl
#  define KMIME_WARN_NON_FOLDING(x) KMIME_WARN << "Non-folding " #x \
          " in header \"" << type() << '\"' << endl
#  define KMIME_WARN_CTL_OUTSIDE_QS(x) KMIME_WARN << "Control character " \
          #x " outside quoted-string in header \"" << type() << "\"" << endl
#  define KMIME_WARN_INVALID_X_IN_Y(X,Y) KMIME_WARN << "Invalid character '" \
          QString(QChar(X)) << "' in " #Y << endl;
#  define KMIME_WARN_TOO_LONG(x) KMIME_WARN << #x \
          " too long or missing delimiter" << endl;
#else
#  define KMIME_NOP do {} while (0)
#  define KMIME_WARN_8BIT(ch) KMIME_NOP
#  define KMIME_WARN_IF_8BIT(ch) KMIME_NOP
#  define KMIME_WARN_PREMATURE_END_OF(x) KMIME_NOP
#  define KMIME_WARN_LONE(x) KMIME_NOP
#  define KMIME_WARN_NON_FOLDING(x) KMIME_NOP
#  define KMIME_WARN_CTL_OUTSIDE_QS(x) KMIME_NOP
#endif

using namespace KMime;
using namespace KMime::Headers;

namespace KMime {
namespace Headers {
//-----<Base>----------------------------------

QCString Base::rfc2047Charset()
{
  if( (e_ncCS==0) || forceCS() )
    return defaultCS();
  else
    return QCString(e_ncCS);
}


void Base::setRFC2047Charset(const QCString &cs)
{
  e_ncCS=cachedCharset(cs);
}


bool Base::forceCS()
{
  return ( p_arent!=0 ? p_arent->forceDefaultCS() : false );
}


QCString Base::defaultCS()
{
  return ( p_arent!=0 ? p_arent->defaultCharset() : Latin1 );
}

// parse the encoded-word (scursor points to after the initial '=')
bool Base::parseEncodedWord( char* & scursor, const char * send,
			     QString & result, QCString & language ) {

  // make sure the caller already did a bit of the work.
  assert( *(scursor-1) == '=' );

  //
  // STEP 1:
  // scan for the charset/language portion of the encoded-word
  //

  char ch = *scursor++;

  if ( ch != '?' ) {
    kdDebug() << "first" << endl;
    KMIME_WARN_PREMATURE_END_OF(EncodedWord);
    return false;
  }

  // remember start of charset (ie. just after the initial "=?") and
  // language (just after the first '*') fields:
  char * charsetStart = scursor;
  char * languageStart = 0;

  // find delimiting '?' (and the '*' separating charset and language
  // tags, if any):
  for ( ; scursor != send ; scursor++ )
    if ( *scursor == '?')
      break;
    else if ( *scursor == '*' && !languageStart ) 
      languageStart = scursor + 1;

  // not found? can't be an encoded-word!
  if ( scursor == send || *scursor != '?' ) {
    kdDebug() << "second" << endl;
    KMIME_WARN_PREMATURE_END_OF(EncodedWord);
    return false;
  }

  // extract the language information, if any (if languageStart is 0,
  // language will be null, too):
  QCString maybeLanguage( languageStart, scursor - languageStart + 1 /*for NUL*/);
  // extract charset information (keep in mind: the size given to the
  // ctor is one off due to the \0 terminator):
  QCString maybeCharset( charsetStart, ( languageStart ? languageStart : scursor + 1 ) - charsetStart );

  //
  // STEP 2:
  // scan for the encoding portion of the encoded-word
  //


  // remember start of encoding (just _after_ the second '?'):
  scursor++;
  char * encodingStart = scursor;

  // find next '?' (ending the encoding tag):
  for ( ; scursor != send ; scursor++ )
    if ( *scursor == '?' ) break;

  // not found? Can't be an encoded-word!
  if ( scursor == send || *scursor != '?' ) {
    kdDebug() << "third" << endl;
    KMIME_WARN_PREMATURE_END_OF(EncodedWord);
    return false;
  }

  // extract the encoding information:
  QCString maybeEncoding( encodingStart, scursor - encodingStart + 1 );


  kdDebug() << "parseEncodedWord: found charset == \"" << maybeCharset
	    << "\"; language == \"" << maybeLanguage
	    << "\"; encoding == \"" << maybeEncoding << "\"" << endl;

  //
  // STEP 3:
  // scan for encoded-text portion of encoded-word
  //


  // remember start of encoded-text (just after the third '?'):
  scursor++;
  char * encodedTextStart = scursor;

  // find next '?' (ending the encoded-text):
  for ( ; scursor != send ; scursor++ )
    if ( *scursor == '?' ) break;

  // not found? Can't be an encoded-word!
  // ### maybe evaluate it nonetheless if the rest is OK?
  if ( scursor == send || *scursor != '?' ) {
    kdDebug() << "fourth" << endl;
    KMIME_WARN_PREMATURE_END_OF(EncodedWord);
    return false;
  }
  scursor++;
  // check for trailing '=':
  if ( scursor == send || *scursor != '=' ) {
    kdDebug() << "fifth" << endl;
    KMIME_WARN_PREMATURE_END_OF(EncodedWord);
    return false;
  }
  scursor++;

  // set end sentinel for encoded-text:
  char * encodedTextEnd = scursor - 2;

  //
  // STEP 4:
  // setup decoders for the transfer encoding and the charset
  //


  // try if there's a codec for the encoding found:
  Codec<> * codec = Codec<>::codecForName( maybeEncoding );
  if ( !codec ) {
    KMIME_WARN_UNKNOWN(Encoding,maybeEncoding);
    return false;
  }

  // get an instance of a corresponding decoder:
  Decoder<> * dec = codec->makeDecoder();
  assert( dec );

  // try if there's a (text)codec for the charset found:
  bool matchOK = false;
  QTextCodec
    *textCodec = KGlobal::charsets()->codecForName( maybeCharset, matchOK );

  if ( !matchOK || !textCodec ) {
    KMIME_WARN_UNKNOWN(Charset,maybeCharset);
    delete dec;
    return false;
  };

  // get an instance of a corresponding (text)decoder:
  QTextDecoder * textDec = textCodec->makeDecoder();
  assert( textDec );

  kdDebug() << "mimeName(): \"" << textCodec->mimeName() << "\"" << endl;

  // allocate a temporary buffer to store chunks of the 8bit text:
  QByteArray buffer( 4096 ); // anyone knows a good size?
  QByteArray::Iterator bit, bend;
  
  //
  // STEP 5:
  // do the actual decoding
  //

  do {
    bit = buffer.begin();
    bend = buffer.end();
    // decode a chunk:
    dec->decode( encodedTextStart, encodedTextEnd, bit, bend );
    // and transform that chunk to Unicode:
    if ( bit - buffer.begin() ) {
      kdDebug() << "chunk: \"" << QCString( buffer.begin(), bit-buffer.begin()+1 ) << "\"" << endl;
      result += textDec->toUnicode( buffer.begin(), bit - buffer.begin() );
      kdDebug() << "result now: \"" << result << "\"" << endl;
    }
    // until nothing was written into the buffer anymore:
  } while ( bit != buffer.begin() );

  result += textDec->toUnicode( 0, 0 ); // flush

  kdDebug() << "result now: \"" << result << "\"" << endl;
  // cleanup:
  delete dec;
  delete textDec;
  language = maybeLanguage;

  return true;
}

//-----</Base>---------------------------------

namespace Generics {

//-----<GUnstructured>-------------------------

void GUnstructured::from7BitString( const QCString & str )
{
  d_ecoded = decodeRFC2047String( str, &e_ncCS, defaultCS(), forceCS() );
}

QCString GUnstructured::as7BitString( bool withHeaderType )
{
  QCString result;
  if ( withHeaderType )
    result = typeIntro();
  result += encodeRFC2047String( d_ecoded, e_ncCS ) ;

  return result;
}

void GUnstructured::fromUnicodeString( const QString & str,
				       const QCString & suggestedCharset )
{
  d_ecoded = str;
  e_ncCS = cachedCharset( suggestedCharset );
}

QString GUnstructured::asUnicodeString()
{
  return d_ecoded;
}

//-----</GUnstructured>-------------------------



//-----<GStructured>-------------------------

static inline void eatWhiteSpace( char* & scursor, const char * send ) {
  while ( scursor != send
	  && ( *scursor == ' ' || *scursor == '\n' ||
	       *scursor == '\t' || *scursor == '\r' ) )
    scursor++;
}

bool GStructured::parseAtom( char * & scursor, const char * send,
			     QString & result, bool allow8Bit )
{
  QPair<char*,int> maybeResult;

  if ( parseAtom( scursor, send, maybeResult, allow8Bit ) ) {
    result += QString::fromLatin1( maybeResult.first, maybeResult.second );
    return true;
  }

  return false;
}

bool GStructured::parseAtom( char * & scursor, const char * send,
			     QPair<char*,int> & result, bool allow8Bit ) {
  bool success = false;
  char * start = scursor;

  while ( scursor != send ) {
    char ch = *scursor++;
    if ( ch > 0 && isAText(ch) ) {
      // AText: OK
      success = true;
    } else if ( allow8Bit && ch < 0 ) {
      // 8bit char: not OK, but be tolerant.
      KMIME_WARN_8BIT(ch);
      success = true;
    } else {
      // CTL or special - marking the end of the atom:
      // re-set sursor to point to the offending
      // char and return:
      scursor--;
      break;
    }
  }
  result.first = start;
  result.second = scursor - start;
  return success;
}

bool GStructured::parseToken( char * & scursor, const char * send,
			      QString & result, bool allow8Bit )
{
  QPair<char*,int> maybeResult;

  if ( parseToken( scursor, send, maybeResult, allow8Bit ) ) {
    result += QString::fromLatin1( maybeResult.first, maybeResult.second );
    return true;
  }

  return false;
}

bool GStructured::parseToken( char * & scursor, const char * send,
			      QPair<char*,int> & result, bool allow8Bit )
{
  bool success = false;
  char * start = scursor;

  while ( scursor != send ) {
    char ch = *scursor++;
    if ( ch > 0 && isTText(ch) ) {
      // TText: OK
      success = true;
    } else if ( allow8Bit && ch < 0 ) {
      // 8bit char: not OK, but be tolerant.
      KMIME_WARN_8BIT(ch);
      success = true;
    } else {
      // CTL or tspecial - marking the end of the atom:
      // re-set sursor to point to the offending
      // char and return:
      scursor--;
      break;
    }
  }
  result.first = start;
  result.second = scursor - start;
  return success;
}

#define READ_ch_OR_FAIL if ( scursor == send ) { \
                          KMIME_WARN_PREMATURE_END_OF(GenericQuotedString); \
                          return false; \
                        } else { \
                          ch = *scursor++; \
		        }

// known issues:
//
// - doesn't handle quoted CRLF

bool GStructured::parseGenericQuotedString( char* & scursor, const char * send,
					    QString & result, bool isCRLF,
					    const char openChar, const char closeChar )
{
  char ch;
  // We are in a quoted-string or domain-literal or comment and the
  // cursor points to the first char after the openChar.
  // We will apply unfolding and quoted-pair removal.
  // We return when we either encounter the end or unescaped openChar
  // or closeChar.

  assert( *(scursor-1) == openChar || *(scursor-1) == closeChar );

  while ( scursor != send ) {
    ch = *scursor++;

    if ( ch == closeChar || ch == openChar ) {
      // end of quoted-string or another opening char:
      // let caller decide what to do.
      return true;
    }

    switch( ch ) {
    case '\\':      // quoted-pair
      // misses "\" CRLF LWSP-char handling, see rfc822, 3.4.5
      READ_ch_OR_FAIL;
      KMIME_WARN_IF_8BIT(ch);
      result += QChar(ch);
      break;
    case '\r':
      // ###
      // The case of lonely '\r' is easy to solve, as they're
      // not part of Unix Line-ending conventions.
      // But I see a problem if we are given Unix-native
      // line-ending-mails, where we cannot determine anymore
      // whether a given '\n' was part of a CRLF or was occuring
      // on it's own.
      READ_ch_OR_FAIL;
      if ( ch != '\n' ) {
	// CR on it's own...
	KMIME_WARN_LONE(CR);
	result += QChar('\r');
	scursor--; // points to after the '\r' again
      } else {
	// CRLF encountered.
	// lookahead: check for folding
	READ_ch_OR_FAIL;
	if ( ch == ' ' || ch == '\t' ) {
	  // correct folding;
	  // position cursor behind the CRLF WSP (unfolding)
	  // and add the WSP to the result
	  result += QChar(ch);
	} else {
	  // this is the "shouldn't happen"-case. There is a CRLF
	  // inside a quoted-string without it being part of FWS.
	  // We take it verbatim.
	  KMIME_WARN_NON_FOLDING(CRLF);
	  result += "\r\n";
	  // the cursor is decremented again, so's we need not
	  // duplicate the whole switch here. "ch" could've been
	  // everything (incl. openChar or closeChar).
	  scursor--;
	}
      }
      break;
    case '\n':
      // Note: CRLF has been handled above already!
      // ### LF needs special treatment, depending on whether isCRLF
      // is true (we can be sure a lonely '\n' was meant this way) or
      // false ('\n' alone could have meant LF or CRLF in the original
      // message. This parser assumes CRLF iff the LF is followed by
      // either WSP (folding) or NULL (premature end of quoted-string;
      // Should be fixed, since NULL is allowed as per rfc822).
      READ_ch_OR_FAIL;
      if ( !isCRLF && ( ch == ' ' || ch == '\t' ) ) {
	// folding
	// correct folding
	result += QChar(ch);
      } else {
	// non-folding
	KMIME_WARN_LONE(LF);
	result += QChar('\n');
	// pos is decremented, so's we need not duplicate the whole
	// switch here. ch could've been everything (incl. <">, "\").
	scursor--;
      }
      break;
    default:
      KMIME_WARN_IF_8BIT(ch);
      result += QChar(ch);
    }
  }

  return false;
}

// known issues:
//
// - doesn't handle encoded-word inside comments.

bool GStructured::parseComment( char* & scursor, const char * send,
				QString & result, bool isCRLF, bool reallySave )
{
  int commentNestingDepth = 1;
  char * afterLastClosingParenPos = 0;
  QString maybeCmnt;
  char * oldscursor = scursor;

  assert( *(scursor-1) == '(' );
  
  while ( commentNestingDepth ) {
    QString cmntPart;
    if ( parseGenericQuotedString( scursor, send, cmntPart, isCRLF, '(', ')' ) ) {
      assert( *(scursor-1) == ')' || *(scursor-1) == '(' );
      // see the kdoc for above function for the possible conditions
      // we have to check:
      switch ( *(scursor-1) ) {
      case ')':
	if ( reallySave ) {
	  // add the chunk that's now surely inside the comment.
	  result += maybeCmnt;
	  result += cmntPart;
	  result += QChar(')');
	  maybeCmnt = QString::null;
	}
	afterLastClosingParenPos = scursor;
	--commentNestingDepth;
	break;
      case '(':
	if ( reallySave ) {
	  // don't add to "result" yet, because we might find that we
	  // are already outside the (broken) comment...
	  maybeCmnt += cmntPart;
	  maybeCmnt += QChar('(');
	}
	++commentNestingDepth;
	break;
      default: assert( 0 );
      } // switch
    } else {
      // !parseGenericQuotedString, ie. premature end
      if ( afterLastClosingParenPos )
	scursor = afterLastClosingParenPos;
      else
	scursor = oldscursor;
      return false;
    }
  } // while

  return true;
}


// known issues: none.

bool GStructured::parsePhrase( char* & scursor, const char * send,
			       QString & result, bool isCRLF )
{
  TokenType found = None;
  QString tmp;
  QCString lang;
  char * successfullyParsed = 0;
  // only used by the encoded-word branch
  char * oldscursor;
  // used to suppress whitespace between adjacent encoded-words
  // (rfc2047, 6.2):
  bool lastWasEncodedWord = false;

  while ( scursor != send ) {
    char ch = *scursor++;
    switch ( ch ) {
    case '"': // quoted-string
      tmp = QString::null;
      if ( parseGenericQuotedString( scursor, send, tmp, isCRLF, '"', '"' ) ) {
	successfullyParsed = scursor;
	assert( *(scursor-1) == '"' );
	switch ( found ) {
	case None:
	  found = QuotedString;
	  break;
	case Phrase:
	case Atom:
	case EncodedWord:
	case QuotedString:
	  found = Phrase;
	  result += QChar(' '); // rfc822, 3.4.4
	  break;
	default:
	  assert( 0 );
	}
	lastWasEncodedWord = false;
	result += tmp;
      } else {
	// premature end of quoted string.
	// What to do? Return leading '"' as special? Return as quoted-string?
	// We do the latter if we already found something, else signal failure.
	if ( found == None ) {
	  return false;
	} else {
	  result += QChar(' '); // rfc822, 3.4.4
	  result += tmp;
	  return true;
	}
      }
      break;
    case '(': // comment
      // parse it, but ignore content:
      tmp = QString::null;
      if ( parseComment( scursor, send, tmp, isCRLF,
			 false /*don't bother with the content*/ ) ) {
	successfullyParsed = scursor;
	lastWasEncodedWord = false; // strictly interpreting rfc2047, 6.2 
      } else {
	if ( found == None )
	  return false;
	else {
	  scursor = successfullyParsed;
	  return true;
	}
      }
      break;
    case '=': // encoded-word
      tmp = QString::null;
      oldscursor = scursor;
      lang = 0;
      if ( parseEncodedWord( scursor, send, tmp, lang ) ) {
	successfullyParsed = scursor;
	switch ( found ) {
	case None:
	  found = EncodedWord;
	  break;
	case Phrase:
	case EncodedWord:
	case Atom:
	case QuotedString:
	  if ( !lastWasEncodedWord )
	    result += QChar(' '); // rfc822, 3.4.4
	  found = Phrase;
	  break;
	default: assert( 0 );
	}
	lastWasEncodedWord = true;
	result += tmp;
	break;
      } else
	// parse as atom:
	scursor = oldscursor;
      // fall though...

    default: //atom
      tmp = QString::null;
      scursor--;
      if ( parseAtom( scursor, send, tmp, true /* allow 8bit */ ) ) {
	successfullyParsed = scursor;
	switch ( found ) {
	case None:
	  found = Atom;
	  break;
	case Phrase:
	case Atom:
	case EncodedWord:
	case QuotedString:
	  found = Phrase;
	  result += QChar(' '); // rfc822, 3.4.4
	  break;
	default:
	  assert( 0 );
	}
	lastWasEncodedWord = false;
	result += tmp;
      } else {
	if ( found == None )
	  return false;
	else {
	  scursor = successfullyParsed;
	  return true;
	}
      }
    }
    eatWhiteSpace( scursor, send );
  }

  return ( found != None );
}


bool GStructured::parseDotAtom( char* & scursor, const char * send,
				QString & result, bool isCRLF )
{
  bool sawInitialAtom = false;
  char * successfullyParsed = 0;
  bool lastSawDot = false;
  QString tmp;

  while ( scursor != send ) {
    char ch = *scursor++;
    switch ( ch ) {
    case '.': // dot in DotAtom
      if ( !sawInitialAtom ) // can't start with a dot!
	return false;
      if ( lastSawDot ) { // ".." is forbidden!
	scursor = successfullyParsed;
	return true;
      }
      lastSawDot = true;
      break;
    case '(': // comment
      // parse it, but ignore content:
      tmp = QString::null;
      if ( !parseComment( scursor, send, tmp, isCRLF,
			 false /*don't bother with the content*/ ) ) {
	if ( !sawInitialAtom )
	  return false;
	else {
	  scursor = successfullyParsed;
	  return true;
	}
      }
      break;
    default: // atom in DotAtom
      if ( !lastSawDot ) {
	if ( sawInitialAtom ) {
	  scursor = successfullyParsed;
	  return true;
	} else
	  return false;
      }
      tmp = QString::null;
      if ( parseAtom( scursor, send, tmp, false /*don't allow 8 bit chars*/ ) ) {
	if ( sawInitialAtom )
	  result += QChar('.');
	sawInitialAtom = true;
	lastSawDot = false;
	result += tmp;
      } else {
	if ( !sawInitialAtom ) {
	  return false;
	} else {
	  scursor = successfullyParsed;
	  return true;
	}
      }
    }
    eatWhiteSpace( scursor, send );
  }

  return sawInitialAtom;
}


// known issues:
//
// 

QString GStructured::getToken( char * & scursor, const char * send,
			       TokenType & tt, bool isCRLF )
{
  while ( scursor != send ) {
    // first, eat any whitespace:
    eatWhiteSpace( scursor, send );
    if ( scursor == send ) break;

    char ch = *scursor++;

    switch ( ch ) {

    case '=': // encoded-word
      
      if ( tt & Phrase ) {
	char * oldscursor = scursor;
	scursor--; // must include the '=' for parsePhrase().
	QString tmp;
	if ( parsePhrase( scursor, send, tmp, isCRLF ) ) {
	  tt = Phrase;
	  return tmp;
	}
	scursor = oldscursor;
      } // else try next possibility:
      if ( tt & EncodedWord ) {
	char * oldscursor = scursor;
	QString tmp;
	QCString lang;
	if ( parseEncodedWord( scursor, send, tmp, lang ) ) {
	  tt = EncodedWord;
	  return tmp;
	}
	scursor = oldscursor;
      } // else try next possibility:
      if ( tt & DotAtom ) {
	char * oldscursor = scursor;
	QString tmp( QChar('=') );
	if ( parseDotAtom( scursor, send, tmp, isCRLF ) ) {
	  tt = DotAtom;
	  return tmp;
	}
	scursor = oldscursor;
      } // else try next possibility:
      if ( tt & Atom ) {
	char * oldscursor = scursor;
	QString tmp( QChar('=') );
	if ( parseAtom( scursor, send, tmp, false /*no 8bit chars*/ ) ) {
	  tt = Atom;
	  return tmp;
	}
	scursor = oldscursor;
      } // else try the last possibility:
      assert( tt & TSpecial ); // must be allowed if the above aren't.
      tt = TSpecial;
      return QString( QChar('=') );
      
      /* ============================================ */
      
    case '"': // quoted-string
      
      if ( tt & Phrase ) {
	char * oldscursor = scursor;
	scursor--; // must include the '"' for parsePhrase().
	QString tmp;
	if ( parsePhrase( scursor, send, tmp, isCRLF ) ) {
	  tt = Phrase;
	  return tmp;
	}
	scursor = oldscursor;
      } // else try next possibility:
      if ( tt & QuotedString ) {
	char * oldscursor = scursor;
	QString tmp;
	if ( parseGenericQuotedString( scursor, send, tmp,
				       isCRLF, '"', '"' ) ) {
	  assert( *(scursor-1) == '"' );
	  tt = QuotedString;
	  return tmp;
	}
	scursor = oldscursor;
      } // else try last possibility:
      assert( tt & (Special|TSpecial) );
      tt = ( tt & Special ) ? Special : TSpecial ;
      return QString( QChar('"') );
      
      /* ============================================ */
      
    case '(': // comment

      {
	char * oldscursor = scursor;
	QString tmp;
	if ( parseComment( scursor, send, tmp, isCRLF, tt & Comment ) ) {
	  if ( tt & Comment ) {
	    // return Comment if asked for...:
	    tt = Comment;
	    return tmp;
	    // ...else ignore it: the only reason why this switch is
	    // wrapped with a while loop:
	  } else break;
	}
	scursor = oldscursor;
      }
      // Parsing failed:
      // return as special or tspecial:
      assert( tt & (Special|TSpecial) );
      tt = ( tt & Special ) ? Special : TSpecial ;
      return QString( QChar('(') );
      
      /* ============================================ */
      
    case '[': // domain-literal
      
      if ( tt & DomainLiteral ) {
	char * oldscursor = scursor;
	QString tmp;
	while ( parseGenericQuotedString( scursor, send,
					  tmp, isCRLF, '[', ']' ) ) {
	  if ( *(scursor-1) == '[' )
	    continue;
	  assert( *(scursor-1) == ']' );
	  tt = DomainLiteral;
	  return tmp;
	}
	scursor = oldscursor;
      }
      assert( tt & (TSpecial|Special) );
      tt = ( tt & Special ) ? Special : TSpecial;
      return QString( QChar('[') );
      
      /* ============================================ */
      
    default: // atom/token or special/tspecial
      if ( tt & Phrase &&
	   ( ch < 0 ||
	     !isTSpecial( ch ) ||
	     isTSpecial( ch ) && !isSpecial( ch ) ) ) {
	char * oldscursor = scursor;
	scursor--; // make parsePhrase() see ch
	QString tmp;
	if ( parsePhrase( scursor, send, tmp, isCRLF ) ) {
	  tt = Phrase;
	  return tmp;
	}
	oldscursor = scursor;
      }
      if ( tt & DotAtom && ch > 0 &&
	   ( !isTSpecial( ch ) || isTSpecial( ch ) && !isSpecial( ch ) ) ) {
	char * oldscursor = scursor;
	scursor--;
	QString tmp;
	if ( parseDotAtom( scursor, send, tmp, isCRLF ) ) {
	  tt = DotAtom;
	  return tmp;
	}
	scursor = oldscursor;
      }
      if ( tt & Atom && ch > 0 &&
	   ( !isTSpecial( ch ) || isTSpecial( ch ) && !isSpecial( ch ) ) ) {
	char * oldscursor = scursor;
	scursor--;
	QString tmp;
	if ( parseAtom( scursor, send, tmp, false /* no 8bit */ ) ) {
	  tt = Atom;
	  return tmp;
	}
	scursor = oldscursor;
      }
      if ( tt & Token && ch > 0 && !isTSpecial( ch ) ) {
	char * oldscursor = scursor;
	QString tmp;
	if ( parseToken( scursor, send, tmp, false /* no 8bit */ ) ) {
	  tt = Token;
	  return tmp;
	}
	scursor = oldscursor;
      }
      if ( tt & Special && isSpecial( ch ) ) {
	tt = Special;
	return QString( QChar(ch) );
      }
      if ( ch < 0 ) {
	tt = EightBit;
	return QString( QChar(ch) );
      }
      assert( isTSpecial( ch ) );
      tt = TSpecial;
      return QString( QChar(ch) );

    }
  }
  tt = None;
  return QString::null;
}

void GStructured::eatCFWS( char* & scursor, const char * send, bool isCRLF ) {
  QString dummy;

  while ( scursor != send ) {
    char * oldscursor = scursor;

    char ch = *scursor++;

    switch( ch ) {
    case ' ':
    case '\t': // whitespace
    case '\r':
    case '\n': // folding
      continue;

    case '(': // comment
      if ( parseComment( scursor, send, dummy, isCRLF, false /*don't save*/ ) )
	continue;
      scursor = oldscursor;
      return;
      
    default:
      scursor = oldscursor;
      return;
    }

  }
}

//-----</GStructured>-------------------------




//-----<GAddress>-------------------------

bool GAddress::parseDomain( char* & scursor, const char * send,
			    QString & result, bool isCRLF ) {
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;
  
  // domain := dot-atom / domain-literal / atom *("." atom)
  //
  // equivalent to:
  // domain = dot-atom / domain-literal,
  // since parseDotAtom does allow CFWS between atoms and dots

  if ( *scursor == '[' ) {
    // domain-literal:
    QString maybeDomainLiteral;
    while ( parseGenericQuotedString( scursor, send, maybeDomainLiteral,
				      isCRLF, '[', ']' ) ) {
      if ( *(scursor-1) == '[' )
	continue;
      assert( *(scursor-1) == ']' );
      result = maybeDomainLiteral;
      return true;
    }
  } else {
    // dot-atom:
    scursor--;
    QString maybeDotAtom;
    if ( parseDotAtom( scursor, send, maybeDotAtom, isCRLF ) ) {
      result = maybeDotAtom;
      return true;
    }
  }
  return false;
}

bool GAddress::parseObsRoute( char* & scursor, const char* send,
			      QStringList & result, bool isCRLF, bool save ) {
  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) return false;

    // empty entry:
    if ( *scursor == ',' ) {
      scursor++;
      if ( save ) result.append( QString::null );
      continue;
    }

    // empty entry ending the list:
    if ( *scursor == ':' ) {
      scursor++;
      if ( save ) result.append( QString::null );
      return true;
    }

    // each non-empty entry must begin with '@':
    if ( *scursor != '@' )
      return false;
    else
      scursor++;

    QString maybeDomain;
    if ( !parseDomain( scursor, send, maybeDomain, isCRLF ) ) return false;
    if ( save ) result.append( maybeDomain );

    // eat the following (optional) comma:
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) return false;
    if ( *scursor == ':' ) { scursor++; return true; }
    if ( *scursor == ',' ) scursor++;

  }

  return false;
}

bool GAddress::parseAddrSpec( char* & scursor, const char * send,
			      AddrSpec & result, bool isCRLF ) {
  //
  // STEP 1:
  // local-part := dot-atom / quoted-string / word *("." word)
  //
  // this is equivalent to:
  // local-part := word *("." word)

  QString maybeLocalPart;
  QString tmp;

  while ( scursor != send ) {
    // first, eat any whitespace
    eatCFWS( scursor, send, isCRLF );

    char ch = *scursor++;
    switch ( ch ) {
    case '.': // dot
      maybeLocalPart += QChar('.');
      break;

    case '@':
      goto SAW_AT_SIGN;
      break;

    case '"': // quoted-string
      tmp = QString::null;
      if ( parseGenericQuotedString( scursor, send, tmp, isCRLF, '"', '"' ) )
	maybeLocalPart += tmp;
      else
	return false;
      break;

    default: // atom
      tmp = QString::null;
      if ( parseAtom( scursor, send, tmp, false /* no 8bit */ ) )
	maybeLocalPart += tmp;
      else
	return false; // parseAtom can only fail if the first char is non-atext.
      break;
    }
  }

  return false;


  //
  // STEP 2:
  // domain
  //

SAW_AT_SIGN:

  assert( *(scursor-1) == '@' );
  
  QString maybeDomain;
  if ( !parseDomain( scursor, send, maybeDomain, isCRLF ) )
    return false;
    
  result.localPart = maybeLocalPart;
  result.domain = maybeDomain;

  return true;
}


bool GAddress::parseAngleAddr( char* & scursor, const char * send,
			       AddrSpec & result, bool isCRLF ) {
  // first, we need an opening angle bracket:
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != '<' ) return false;

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  if ( *scursor == '@' || *scursor == ',' ) {
    // obs-route: parse, but ignore:
    scursor--;
    QStringList dummy;
    if ( !parseObsRoute( scursor, send, dummy,
			 isCRLF, false /* don't save */ ) )
      return false;
  }

  // parse addr-spec:
  AddrSpec maybeAddrSpec;
  if ( !parseAddrSpec( scursor, send, maybeAddrSpec, isCRLF ) ) return false;

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != '>' ) return false;
  *scursor++;
  
  result = maybeAddrSpec;
  return true;

}

bool GAddress::parseMailbox( char* & scursor, const char * send,
			     Mailbox & result, bool isCRLF ) {

  // mailbox := addr-spec / ([ display-name ] angle-addr)

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  AddrSpec maybeAddrSpec;

  // first, try if it's a vanilla addr-spec:
  char * oldscursor = scursor;
  if ( parseAddrSpec( scursor, send, maybeAddrSpec, isCRLF ) ) {
    result.displayName = QString::null;
    result.addrSpec = maybeAddrSpec;
    return true;
  }
  scursor = oldscursor;

  // second, see if there's a display-name:
  QString maybeDisplayName;
  if ( !parsePhrase( scursor, send, maybeDisplayName, isCRLF ) ) {
    // failed: reset cursor, note absent display-name
    maybeDisplayName = QString::null;
    scursor = oldscursor;
  } else {
    // succeeded: eat CFWS
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) return false;
  }

  // third, parse the angle-addr:
  if ( !parseAngleAddr( scursor, send, maybeAddrSpec, isCRLF ) )
    return false;

  result.displayName = maybeDisplayName;
  result.addrSpec = maybeAddrSpec;
  return true;
}

bool GAddress::parseGroup( char* & scursor, const char * send,
			   Address & result, bool isCRLF ) {
  // group         := display-name ":" [ mailbox-list / CFWS ] ";" [CFWS]
  //
  // equivalent to:
  // group   := display-name ":" [ obs-mbox-list ] ";"

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  // get display-name:
  QString maybeDisplayName;
  if ( !parsePhrase( scursor, send, maybeDisplayName, isCRLF ) )
    return false;

  // get ":":
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != ':' ) return false;

  result.displayName = maybeDisplayName;

  // get obs-mbox-list (may contain empty entries):
  scursor++;
  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) return false;
    
    // empty entry:
    if ( *scursor == ',' ) { scursor++; continue; }

    // empty entry ending the list:
    if ( *scursor == ';' ) { scursor++; return true; }

    Mailbox maybeMailbox;
    if ( !parseMailbox( scursor, send, maybeMailbox, isCRLF ) )
      return false;
    result.mailboxList.append( maybeMailbox );
    
    eatCFWS( scursor, send, isCRLF );
    // premature end:
    if ( scursor == send ) return false;
    // regular end of the list:
    if ( *scursor == ';' ) { scursor++; return true; }
    // eat regular list entry separator:
    if ( *scursor == ',' ) scursor++;
  }
  return false;
}


bool GAddress::parseAddress( char* & scursor, const char * send,
			     Address & result, bool isCRLF ) {
  // address       := mailbox / group

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  // first try if it's a single mailbox:
  Mailbox maybeMailbox;
  char * oldscursor = scursor;
  if ( parseMailbox( scursor, send, maybeMailbox, isCRLF ) ) {
    // yes, it is:
    result.displayName = QString::null;
    result.mailboxList.append( maybeMailbox );
    return true;
  }
  scursor = oldscursor;

  Address maybeAddress;
  
  // no, it's not a single mailbox. Try if it's a group:
  if ( !parseGroup( scursor, send, maybeAddress, isCRLF ) )
    return false;

  result = maybeAddress;
  return true;
}

bool GAddress::parseAddressList( char* & scursor, const char * send,
				 QValueList<Address> & result, bool isCRLF ) {
  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // end of header: this is OK.
    if ( scursor != send ) return true;
    // empty entry: ignore:
    if ( *scursor == ',' ) { scursor++; continue; }

    // parse one entry
    Address maybeAddress;
    if ( !parseAddress( scursor, send, maybeAddress, isCRLF ) ) return false;
    result.append( maybeAddress );

    eatCFWS( scursor, send, isCRLF );
    // end of header: this is OK.
    if ( scursor == send ) return true;
    // comma separating entries: eat it.
    if ( *scursor == ',' ) scursor++;
  }
  return true;
}

//-----</GAddress>-------------------------



//-----<MailboxList>-------------------------

bool MailboxList::parse( char* & scursor, const char * send, bool isCRLF ) {
  // examples:
  // from := "From:" mailbox-list CRLF
  // sender := "Sender:" mailbox CRLF

  // parse an address-list:
  QValueList<Address> maybeAddressList;
  if ( !parseAddressList( scursor, send, maybeAddressList, isCRLF ) )
    return false;

  mMailboxList.clear();

  // extract the mailboxes and complain if there are groups:
  QValueList<Address>::Iterator it;
  for ( it = maybeAddressList.begin(); it != maybeAddressList.end() ; ++it ) {
    if ( !(*it).displayName.isEmpty() ) {
      KMIME_WARN << "mailbox groups in header disallowing them! Name: \""
		 << (*it).displayName << "\"" << endl;
    }
    mMailboxList += (*it).mailboxList;
  }
  return true;
}

//-----</MailboxList>-------------------------



//-----<SingleMailbox>-------------------------

bool SingleMailbox::parse( char* & scursor, const char * send, bool isCRLF ) {
  if ( !MailboxList::parse( scursor, send, isCRLF ) ) return false;

  if ( mMailboxList.count() != 1 ) {
    KMIME_WARN << "multiple mailboxes in header allowing only a single one!"
	       << endl;
  }
  return true;
}

//-----</SingleMailbox>-------------------------



//-----<AddressList>-------------------------

bool AddressList::parse( char* & scursor, const char * send, bool isCRLF ) {

  QValueList<Address> maybeAddressList;
  if ( !parseAddressList( scursor, send, maybeAddressList, isCRLF ) )
    return false;

  mAddressList = maybeAddressList;
  return true;
}

//-----</AddressList>-------------------------



//-----<GToken>-------------------------

bool GToken::parse( char* & scursor, const char * send, bool isCRLF ) {

  eatCFWS( scursor, send, isCRLF );
  // must not be empty:
  if ( scursor == send ) return false;

  QPair<char*,int> maybeToken;
  if ( !parseToken( scursor, send, maybeToken, false /* no 8bit chars */ ) )
    return false;
  mToken = QCString( maybeToken.first, maybeToken.second );

  // complain if trailing garbage is found:
  eatCFWS( scursor, send, isCRLF );
  if ( scursor != send ) {
    KMIME_WARN << "trailing garbage after token in header allowing "
      "only a single token!" << endl;
  }
  return true;
}

//-----</GToken>-------------------------



//-----<GPhraseList>-------------------------

bool GPhraseList::parse( char* & scursor, const char * send, bool isCRLF ) {

  mPhraseList.clear();

  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // empty entry ending the list: OK.
    if ( scursor == send ) return true;
    // empty entry: ignore.
    if ( *scursor != ',' ) { scursor++; continue; }

    QString maybePhrase;
    if ( !parsePhrase( scursor, send, maybePhrase, isCRLF ) )
      return false;
    mPhraseList.append( maybePhrase );

    eatCFWS( scursor, send, isCRLF );
    // non-empty entry ending the list: OK.
    if ( scursor == send ) return true;
    // comma separating the phrases: eat.
    if ( *scursor != ',' ) scursor++;
  }
  return true;
}

//-----</GPhraseList>-------------------------



//-----<GDotAtom>-------------------------

bool GDotAtom::parse( char* & scursor, const char * send, bool isCRLF ) {

  QString maybeDotAtom;
  if ( !parseDotAtom( scursor, send, maybeDotAtom, isCRLF ) )
    return false;
  
  mDotAtom = maybeDotAtom;

  eatCFWS( scursor, send, isCRLF );
  if ( scursor != send ) {
    KMIME_WARN << "trailing garbage after dot-atom in header allowing "
      "only a single dot-atom!" << endl;
  }
  return true;
}

//-----</GDotAtom>-------------------------



static QString asterisk = QString::fromLatin1("*0*",1);
static QString asteriskZero = QString::fromLatin1("*0*",2);
//static QString asteriskZeroAsterisk = QString::fromLatin1("*0*",3);

//-----<GParametrized>-------------------------

bool GParametrized::parseParameter( char* & scursor, const char * send,
				    QPair<QString,QStringOrQPair> & result,
				    bool isCRLF ) {
  // parameter = regular-parameter / extended-parameter
  // regular-parameter = regular-parameter-name "=" value
  // extended-parameter = 
  // value = token / quoted-string
  //
  // note that rfc2231 handling is out of the scope of this function.
  // Therefore we return the attribute as QString and the value as
  // (start,length) tupel if we see that the value is encoded
  // (trailing asterisk), for parseParameterList to decode...

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  //
  // parse the parameter name:
  //
  QString maybeAttribute;
  if ( !parseToken( scursor, send, maybeAttribute, false /* no 8bit */ ) )
    return false;

  eatCFWS( scursor, send, isCRLF );
  // premature end: not OK (haven't seen '=' yet).
  if ( scursor == send || *scursor != '=' ) return false;
  scursor++; // eat '='

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    // don't choke on attribute=, meaning the value was omitted:
    if ( maybeAttribute.endsWith( asterisk ) ) {
      KMIME_WARN << "attribute ends with \"*\", but value is empty! "
	"Chopping away \"*\"." << endl;
      maybeAttribute.truncate( maybeAttribute.length() - 1 );
    }
    result = qMakePair( maybeAttribute.lower(), QStringOrQPair() );
    return true;
  }

  char * oldscursor = scursor;

  //
  // parse the parameter value:
  //
  QStringOrQPair maybeValue;
  if ( *scursor == '"' ) {
    // value is a quoted-string:
    scursor++;
    if ( maybeAttribute.endsWith( asterisk ) ) {
      // attributes ending with "*" designate extended-parameters,
      // which cannot have quoted-strings as values. So we remove the
      // trailing "*" to not confuse upper layers.
      KMIME_WARN << "attribute ends with \"*\", but value is a quoted-string! "
	"Chopping away \"*\"." << endl;
      maybeAttribute.truncate( maybeAttribute.length() - 1 );
    }

    if ( !parseGenericQuotedString( scursor, send, maybeValue.qstring, isCRLF ) ) {
      scursor = oldscursor;
      result = qMakePair( maybeAttribute.lower(), QStringOrQPair() );
      return false; // this case needs further processing by upper layers!!
    }
  } else {
    // value is a token:
    if ( !parseToken( scursor, send, maybeValue.qpair, false /* no 8bit */ ) ) {
      scursor = oldscursor;
      result = qMakePair( maybeAttribute.lower(), QStringOrQPair() );
      return false; // this case needs further processing by upper layers!!
    }
  }

  result = qMakePair( maybeAttribute.lower(), maybeValue );
  return true;
}



bool GParametrized::parseRawParameterList( char* & scursor, const char * send,
					   QMap<QString,QStringOrQPair> & result,
					   bool isCRLF ) {
  // we use parseParameter() consecutively to obtain a map of raw
  // attributes to raw values. "Raw" here means that we don't do
  // rfc2231 decoding and concatenation. This is left to
  // parseParameterList(), which will call this function.
  //
  // The main reason for making this chunk of code a separate
  // (private) method is that we can deal with broken parameters
  // _here_ and leave the rfc2231 handling solely to
  // parseParameterList(), which will still be enough work.

  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // empty entry ending the list: OK.
    if ( scursor == send ) return true;
    // empty list entry: ignore.
    if ( *scursor == ';' ) { scursor++; continue; }

    QPair<QString,QStringOrQPair> maybeParameter;
    if ( !parseParameter( scursor, send, maybeParameter, isCRLF ) ) {
      // we need to do a bit of work if the attribute is not
      // NULL. These are the cases marked with "needs further
      // processing" in parseParameter(). Specifically, parsing of the
      // token or the quoted-string, which should represent the value,
      // failed. We take the easy way out and simply search for the
      // next ';' to start parsing again. (Another option would be to
      // take the text between '=' and ';' as value)
      if ( maybeParameter.first.isNull() ) return false;
      while ( scursor != send ) {
	if ( *scursor++ == ';' ) goto IS_SEMICOLON;
      }
      // scursor == send case: end of list.
      return true;
    IS_SEMICOLON:
      // *scursor == ';' case: parse next entry.
      continue;
    }
    // successful parsing brings us here:
    result.insert( maybeParameter.first, maybeParameter.second );

    eatCFWS( scursor, send, isCRLF );
    // end of header: ends list.
    if ( scursor == send ) return true;
    // regular separator: eat it.
    if ( *scursor == ';' ) scursor++;
  }
  return true;
}


static void decodeRFC2231Value( Codec<>* & rfc2231Codec,
				QTextCodec* & textcodec,
				bool isContinuation, QString & value,
				QPair<char*,int> & source ) {

  // 
  // parse the raw value into (charset,language,text):
  // 

  char * decBegin = source.first;
  char * decCursor = decBegin;
  char * decEnd = decCursor + source.second;

  if ( !isContinuation ) {
    // find the first single quote
    while ( decCursor != decEnd ) {
      if ( *decCursor == '\'' ) break;
      else decCursor++;
    }
    
    if ( decCursor == decEnd ) {
      // there wasn't a single single quote at all!
      // take the whole value to be in latin-1:
      KMIME_WARN << "No charset in extended-initial-value. "
	"Assuming \"iso-8859-1\"." << endl;
      value += QString::fromLatin1( decBegin, source.second );
      return;
    }
    
    QCString charset( decBegin, decCursor - decBegin );
    
    char * oldDecCursor = ++decCursor;
    // find the second single quote (we ignore the language tag):
    while ( decCursor != decEnd ) {
      if ( *decCursor == '\'' ) break;
      else decCursor++;
    }
    if ( decCursor == decEnd ) {
      KMIME_WARN << "No language in extended-initial-value. "
	"Trying to recover." << endl;
      decCursor = oldDecCursor;
    } else
      decCursor++;

    // decCursor now points to the start of the
    // "extended-other-values":

    //
    // get the decoders:
    //

    bool matchOK = false;
    textcodec = KGlobal::charsets()->codecForName( charset, matchOK );
    if ( !matchOK ) {
      textcodec = 0;
      KMIME_WARN_UNKNOWN(Charset,charset);
    }
  }

  if ( !rfc2231Codec ) {
    rfc2231Codec = Codec<>::codecForName("x-kmime-rfc2231");
    assert( rfc2231Codec );
  }

  if ( !textcodec ) {
    value += QString::fromLatin1( decCursor, decEnd - decCursor );
    return;
  }
  
  QTextDecoder * textDec = textcodec->makeDecoder();
  assert( textDec );
  
  Decoder<> * dec = rfc2231Codec->makeDecoder();
  assert( dec );
  
  //  
  // do the decoding:
  //

  QByteArray buffer( decEnd - decCursor );
  QByteArray::Iterator bit, bend;

  do {
    bit = buffer.begin();
    bend = buffer.end();
    // decode a chunk:
    dec->decode( decCursor, decEnd, bit, bend );
    // and transform that chunk to Unicode:
    if ( bit - buffer.begin() ) {
      kdDebug() << "chunk: \"" << QCString( buffer.begin(), bit-buffer.begin()+1 ) << "\"" << endl;
      value += textDec->toUnicode( buffer.begin(), bit - buffer.begin() );
      kdDebug() << "value now: \"" << value << "\"" << endl;
    }
    // until we reach the end and nothing was written into the buffer
    // anymore:
  } while ( decCursor != decEnd || bit != buffer.begin() );
  
  value += textDec->toUnicode( 0, 0 ); // flush
  
  kdDebug() << "value now: \"" << value << "\"" << endl;
  // cleanup:
  delete dec;
  delete textDec;
}



bool GParametrized::parseParameterList( char* & scursor, const char * send,
					QMap<QString,QString> & result,
					bool isCRLF ) {
  // parse the list into raw attribute-value pairs:
  QMap<QString,QStringOrQPair> rawParameterList;
  if (!parseRawParameterList( scursor, send, rawParameterList, isCRLF ) )
    return false;

  if ( rawParameterList.isEmpty() ) return true;

  // decode rfc 2231 continuations and alternate charset encoding:

  // NOTE: this code assumes that what QMapIterator delivers is sorted
  // by the key!

  Codec<> * rfc2231Codec = 0;
  QTextCodec * textcodec = 0;
  QString attribute;
  QString value;
  enum { NoMode = 0x0, Continued = 0x1, Encoded = 0x2 } mode;

  QMapIterator<QString,QStringOrQPair> it, end = rawParameterList.end();

  for ( it = rawParameterList.begin() ; it != end ; ++it ) {
    if ( attribute.isNull() || !it.key().startsWith( attribute ) ) {
      //
      // new attribute:
      //

      // store the last attribute/value pair in the result map now:
      if ( !attribute.isNull() ) result.insert( attribute, value );
      // and extract the information from the new raw attribute:
      value = QString::null;
      attribute = it.key();
      mode = NoMode;
      // is the value encoded?
      if ( attribute.endsWith( asterisk ) ) {
	attribute.truncate( attribute.length() - 1 );
	(int)mode |= Encoded;
      }
      // is the value continued?
      if ( attribute.endsWith( asteriskZero ) ) {
	attribute.truncate( attribute.length() - 2 );
	(int)mode |= Continued;
      }
      //
      // decode if necessary:
      //
      if ( mode & Encoded ) {
	decodeRFC2231Value( rfc2231Codec, textcodec,
			    false, /* isn't continuation */
			    value, (*it).qpair );
      } else {
	// not encoded.
	value += (*it).qstring;
      }

      //
      // shortcut-processing when the value isn't encoded:
      //

      if ( !(mode & Continued) ) {
	// save result already:
	result.insert( attribute, value );
	// force begin of a new attribute:
	attribute = QString::null;
      }
    } else /* it.key().startsWith( attribute ) */ {
      //
      // continuation
      //
      
      // ignore the section and trust QMap to have sorted the keys:
      if ( it.key().endsWith( asterisk ) )
	// encoded
	decodeRFC2231Value( rfc2231Codec, textcodec,
			    true, /* is continuation */
			    value, (*it).qpair );
      else
	// not encoded
	value += (*it).qstring;
    }
  }

  return true;
}

//-----</GParametrized>-------------------------




//-----</GContentType>-------------------------

bool GContentType::parse( char* & scursor, const char * send, bool isCRLF ) {

  // content-type: type "/" subtype *(";" parameter)

  mMimeType = 0;
  mMimeSubType = 0;
  mParameterHash.clear();

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    // empty header
    return false;
  }

  //
  // type
  // 

  QPair<char*,int> maybeMimeType;
  if ( !parseToken( scursor, send, maybeMimeType, false /* no 8Bit */ ) )
    return false;

  mMimeType = QCString( maybeMimeType.first, maybeMimeType.second ).lower();

  //
  // subtype
  //

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != '/' ) return false;
  scursor++;
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  QPair<char*,int> maybeSubType;
  if ( !parseToken( scursor, send, maybeSubType, false /* no 8bit */ ) )
    return false;

  mMimeSubType = QCString( maybeSubType.first, maybeSubType.second ).lower();

  //
  // parameter list
  //

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return true; // no parameters
  
  if ( *scursor != ';' ) return false;
  scursor++;
  
  if ( !parseParameterList( scursor, send, mParameterHash, isCRLF ) )
    return false;

  return true;
}

//-----</GContentType>-------------------------



//-----<GTokenWithParameterList>-------------------------

bool GCISTokenWithParameterList::parse( char* & scursor, const char * send, bool isCRLF ) {

  mToken = 0;
  mParameterHash.clear();

  //
  // token
  //

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;
  
  QPair<char*,int> maybeToken;
  if ( !parseToken( scursor, send, maybeToken, false /* no 8Bit */ ) )
    return false;

  mToken = QCString( maybeToken.first, maybeToken.second ).lower();

  //
  // parameter list
  //

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return true; // no parameters
  
  if ( *scursor != ';' ) return false;
  scursor++;
  
  if ( !parseParameterList( scursor, send, mParameterHash, isCRLF ) )
    return false;

  return true;
}

//-----</GTokenWithParameterList>-------------------------



//-----<GIdent>-------------------------

bool GIdent::parse( char* & scursor, const char * send, bool isCRLF ) {

  // msg-id   := "<" id-left "@" id-right ">"
  // id-left  := dot-atom-text / no-fold-quote / local-part
  // id-right := dot-atom-text / no-fold-literal / domain
  //
  // equivalent to:
  // msg-id   := angle-addr

  mMsgIdList.clear();

  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // empty entry ending the list: OK.
    if ( scursor == send ) return true;
    // empty entry: ignore.
    if ( *scursor == ',' ) { scursor++; continue; }

    AddrSpec maybeMsgId;
    if ( !parseAngleAddr( scursor, send, maybeMsgId, isCRLF ) )
      return false;
    mMsgIdList.append( maybeMsgId );

    eatCFWS( scursor, send, isCRLF );
    // header end ending the list: OK.
    if ( scursor == send ) return true;
    // regular item separator: eat it.
    if ( *scursor == ',' ) scursor++;
  }
  return true;
}

//-----</GIdent>-------------------------



//-----<GSingleIdent>-------------------------

bool GSingleIdent::parse( char* & scursor, const char * send, bool isCRLF ) {

  if ( !GIdent::parse( scursor, send, isCRLF ) ) return false;

  if ( mMsgIdList.count() > 1 ) {
    KMIME_WARN << "more than one msg-id in header "
      "allowing only a single one!" << endl;
  }
  return true;
}

//-----</GSingleIdent>-------------------------




}; // namespace Generics


//-----<ReturnPath>-------------------------

bool ReturnPath::parse( char* & scursor, const char * send, bool isCRLF ) {
  
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  char * oldscursor = scursor;

  Mailbox maybeMailbox;
  if ( !parseMailbox( scursor, send, maybeMailbox, isCRLF ) ) {
    // mailbox parsing failed, but check for empty brackets:
    scursor = oldscursor;
    if ( *scursor != '<' ) return false;
    scursor++;
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send || *scursor != '>' ) return false;
    scursor++;
    
    // prepare a Null mailbox:
    AddrSpec emptyAddrSpec;
    maybeMailbox.displayName = QString::null;
    maybeMailbox.addrSpec = emptyAddrSpec;
  } else
    // check that there was no display-name:
    if ( !maybeMailbox.displayName.isEmpty() ) {
    KMIME_WARN << "display-name \"" << maybeMailbox.displayName
	       << "\" in Return-Path!" << endl;
  }

  // see if that was all:
  eatCFWS( scursor, send, isCRLF );
  // and warn if it wasn't:
  if ( scursor != send ) {
    KMIME_WARN << "trailing garbage after angle-addr in Return-Path!" << endl;
  }
  return true;
}

//-----</ReturnPath>-------------------------




//-----<Generic>-------------------------------

void Generic::setType(const char *type)
{
  if(t_ype)
    delete[] t_ype;
  if(type) {
    t_ype=new char[strlen(type)+1];
    strcpy(t_ype, type);
  }
  else
    t_ype=0;
}

//-----<Generic>-------------------------------


#if !defined(KMIME_NEW_STYLE_CLASSTREE)
//-----<MessageID>-----------------------------

void MessageID::from7BitString(const QCString &s)
{
  m_id=s;
}


QCString MessageID::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+m_id );
  else
    return m_id;
}


void MessageID::fromUnicodeString(const QString &s, const QCString&)
{
  m_id=s.latin1(); //Message-Ids can only contain us-ascii chars
}


QString MessageID::asUnicodeString()
{
  return QString::fromLatin1(m_id);
}


void MessageID::generate(const QCString &fqdn)
{
  m_id="<"+uniqueString()+"@"+fqdn+">";
}

//-----</MessageID>----------------------------
#endif


//-----<Control>-------------------------------

void Control::from7BitString(const QCString &s)
{
  c_trlMsg=s;
}


QCString Control::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+c_trlMsg );
  else
    return c_trlMsg;
}


void Control::fromUnicodeString(const QString &s, const QCString&)
{
  c_trlMsg=s.latin1();
}


QString Control::asUnicodeString()
{
  return QString::fromLatin1(c_trlMsg);
}

//-----</Control>------------------------------



#if !defined(KMIME_NEW_STYLE_CLASSTREE)
//-----<AddressField>--------------------------
void AddressField::from7BitString(const QCString &s)
{
  int pos1=0, pos2=0, type=0;
  QCString n;

  //so what do we have here ?
  if(s.find( QRegExp("*@*(*)", false, true) )!=-1) type=2;       // From: foo@bar.com (John Doe)
  else if(s.find( QRegExp("*<*@*>", false, true) )!=-1) type=1;  // From: John Doe <foo@bar.com>
  else if(s.find( QRegExp("*@*", false, true) )!=-1) type=0;     // From: foo@bar.com
  else { //broken From header => just decode it
    n_ame=decodeRFC2047String(s, &e_ncCS, defaultCS(), forceCS());
    return;
  }

  switch(type) {

    case 0:
      e_mail=s.copy();
    break;

    case 1:
      pos1=0;
      pos2=s.find('<');
      if(pos2!=-1) {
        n=s.mid(pos1, pos2-pos1).stripWhiteSpace();
        pos1=pos2+1;
        pos2=s.find('>', pos1);
        if(pos2!=-1)
          e_mail=s.mid(pos1, pos2-pos1);
      }
      else return;
    break;

    case 2:
      pos1=0;
      pos2=s.find('(');
      if(pos2!=-1) {
        e_mail=s.mid(pos1, pos2-pos1).stripWhiteSpace();
        pos1=pos2+1;
        pos2=s.find(')', pos1);
        if(pos2!=-1)
          n=s.mid(pos1, pos2-pos1).stripWhiteSpace();
      }
    break;

    default: break;
  }

  if(!n.isEmpty()) {
    removeQuots(n);
    n_ame=decodeRFC2047String(n, &e_ncCS, defaultCS(), forceCS());
  }
}


QCString AddressField::as7BitString(bool incType)
{
  QCString ret;

  if(incType && type()[0]!='\0')
    ret=typeIntro();

  if(n_ame.isEmpty())
    ret+=e_mail;
  else {
    if (isUsAscii(n_ame)) {
      QCString tmp(n_ame.latin1());
      addQuotes(tmp, false);
      ret+=tmp;
    } else {
      ret+=encodeRFC2047String(n_ame, e_ncCS, true);
    }
    if (!e_mail.isEmpty())
      ret += " <"+e_mail+">";
  }

  return ret;
}


void AddressField::fromUnicodeString(const QString &s, const QCString &cs)
{
  int pos1=0, pos2=0, type=0;
  QCString n;

  e_ncCS=cachedCharset(cs);

  //so what do we have here ?
  if(s.find( QRegExp("*@*(*)", false, true) )!=-1) type=2;       // From: foo@bar.com (John Doe)
  else if(s.find( QRegExp("*<*@*>", false, true) )!=-1) type=1;  // From: John Doe <foo@bar.com>
  else if(s.find( QRegExp("*@*", false, true) )!=-1) type=0;     // From: foo@bar.com
  else { //broken From header => just copy it
    n_ame=s;
    return;
  }

  switch(type) {

    case 0:
      e_mail=s.latin1();
    break;

    case 1:
      pos1=0;
      pos2=s.find('<');
      if(pos2!=-1) {
        n_ame=s.mid(pos1, pos2-pos1).stripWhiteSpace();
        pos1=pos2+1;
        pos2=s.find('>', pos1);
        if(pos2!=-1)
          e_mail=s.mid(pos1, pos2-pos1).latin1();
      }
      else return;
    break;

    case 2:
      pos1=0;
      pos2=s.find('(');
      if(pos2!=-1) {
        e_mail=s.mid(pos1, pos2-pos1).stripWhiteSpace().latin1();
        pos1=pos2+1;
        pos2=s.find(')', pos1);
        if(pos2!=-1)
          n_ame=s.mid(pos1, pos2-pos1).stripWhiteSpace();
      }
    break;

    default: break;
  }

  if(!n_ame.isEmpty())
    removeQuots(n_ame);
}


QString AddressField::asUnicodeString()
{
  if(n_ame.isEmpty())
    return QString(e_mail);
  else {
    QString s = n_ame;
    if (!e_mail.isEmpty())
      s += " <"+e_mail+">";
    return s;
  }
}


QCString AddressField::nameAs7Bit()
{
  return encodeRFC2047String(n_ame, e_ncCS);
}


void AddressField::setNameFrom7Bit(const QCString &s)
{
  n_ame=decodeRFC2047String(s, &e_ncCS, defaultCS(), forceCS());
}

//-----</AddressField>-------------------------
#endif


//-----<MailCopiesTo>--------------------------

bool MailCopiesTo::isValid()
{
  if (hasEmail())
    return true;

  if ((n_ame == "nobody") ||
      (n_ame == "never") ||
      (n_ame == "poster") ||
      (n_ame == "always"))
    return true;
  else
    return false;
}


bool MailCopiesTo::alwaysCopy()
{
  return (hasEmail() || (n_ame == "poster") || (n_ame == "always"));
}


bool MailCopiesTo::neverCopy()
{
  return ((n_ame == "nobody") || (n_ame == "never"));
}

//-----</MailCopiesTo>-------------------------




//-----<Date>----------------------------------

void Date::from7BitString(const QCString &s)
{
  t_ime=KRFCDate::parseDate(s);
}


QCString Date::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+KRFCDate::rfc2822DateString(t_ime) );
  else
    return QCString(KRFCDate::rfc2822DateString(t_ime));
}


void Date::fromUnicodeString(const QString &s, const QCString&)
{
  from7BitString( QCString(s.latin1()) );
}


QString Date::asUnicodeString()
{
  return QString::fromLatin1(as7BitString(false));
}


QDateTime Date::qdt()
{
  QDateTime dt;
  dt.setTime_t(t_ime);
  return dt;
}


int Date::ageInDays()
{
  QDate today=QDate::currentDate();
  return ( qdt().date().daysTo(today) );
}

//-----</Date>---------------------------------



#if !defined(KMIME_NEW_STYLE_CLASSTREE)
//-----<To>------------------------------------

void To::from7BitString(const QCString &s)
{
  if(a_ddrList)
    a_ddrList->clear();
  else {
    a_ddrList=new QPtrList<AddressField>;
    a_ddrList->setAutoDelete(true);
  }

  KQCStringSplitter split;
  split.init(s, ",");
  bool splitOk=split.first();
  if(!splitOk)
    a_ddrList->append( new AddressField(p_arent, s ));
  else {
    do {
      a_ddrList->append( new AddressField(p_arent, split.string()) );
    } while(split.next());
  }

  e_ncCS=cachedCharset(a_ddrList->first()->rfc2047Charset());
}


QCString To::as7BitString(bool incType)
{
  QCString ret;

  if(incType)
    ret+=typeIntro();

  if (a_ddrList) {
    AddressField *it=a_ddrList->first();
    if (it)
      ret+=it->as7BitString(false);
    for (it=a_ddrList->next() ; it != 0; it=a_ddrList->next() )
      ret+=","+it->as7BitString(false);
  }

  return ret;
}


void To::fromUnicodeString(const QString &s, const QCString &cs)
{
  if(a_ddrList)
    a_ddrList->clear();
  else  {
    a_ddrList=new QPtrList<AddressField>;
    a_ddrList->setAutoDelete(true);
  }

  QStringList l=QStringList::split(",", s);

  QStringList::Iterator it=l.begin();
  for(; it!=l.end(); ++it)
    a_ddrList->append(new AddressField( p_arent, (*it), cs ));

  e_ncCS=cachedCharset(cs);
}


QString To::asUnicodeString()
{
  if(!a_ddrList)
    return QString::null;

  QString ret;
  AddressField *it=a_ddrList->first();

  if (it)
    ret+=it->asUnicodeString();
  for (it=a_ddrList->next() ; it != 0; it=a_ddrList->next() )
    ret+=","+it->asUnicodeString();
  return ret;
}


void To::addAddress(const AddressField &a)
{
  if(!a_ddrList) {
    a_ddrList=new QPtrList<AddressField>;
    a_ddrList->setAutoDelete(true);
  }

  AddressField *add=new AddressField(a);
  add->setParent(p_arent);
  a_ddrList->append(add);
}


void To::emails(QStrList *l)
{
  l->clear();

  for (AddressField *it=a_ddrList->first(); it != 0; it=a_ddrList->next() )
    if( it->hasEmail() )
      l->append( it->email() );
}

//-----</To>-----------------------------------
#endif


//-----<Newsgroups>----------------------------

void Newsgroups::from7BitString(const QCString &s)
{
  g_roups=s;
  e_ncCS=cachedCharset("UTF-8");
}


QCString Newsgroups::as7BitString(bool incType)
{
  if(incType)
    return (typeIntro()+g_roups);
  else
    return g_roups;
}


void Newsgroups::fromUnicodeString(const QString &s, const QCString&)
{
  g_roups=s.utf8();
  e_ncCS=cachedCharset("UTF-8");
}


QString Newsgroups::asUnicodeString()
{
  return QString::fromUtf8(g_roups);
}


QCString Newsgroups::firstGroup()
{
  int pos=0;
  if(!g_roups.isEmpty()) {
    pos=g_roups.find(',');
    if(pos==-1)
      return g_roups;
    else
      return g_roups.left(pos);
  }
  else
    return QCString();
}


QStringList Newsgroups::getGroups()
{
  QStringList temp = QStringList::split(',', g_roups);
  QStringList ret;
  QString s;

  for (QStringList::Iterator it = temp.begin(); it != temp.end(); ++it ) {
    s = (*it).simplifyWhiteSpace();
    ret.append(s);
  }

  return ret;
}

//-----</Newsgroups>---------------------------



//-----<Lines>---------------------------------

void Lines::from7BitString(const QCString &s)
{
  l_ines=s.toInt();
  e_ncCS=cachedCharset(Latin1);
}


QCString Lines::as7BitString(bool incType)
{
  QCString num;
  num.setNum(l_ines);

  if(incType)
    return ( typeIntro()+num );
  else
    return num;
}


void Lines::fromUnicodeString(const QString &s, const QCString&)
{
  l_ines=s.toInt();
  e_ncCS=cachedCharset(Latin1);
}


QString Lines::asUnicodeString()
{
  QString num;
  num.setNum(l_ines);

  return num;
}

//-----</Lines>--------------------------------



#if !defined(KMIME_NEW_STYLE_CLASSTREE)
//-----<References>----------------------------

void References::from7BitString(const QCString &s)
{
  r_ef=s;
  e_ncCS=cachedCharset(Latin1);
}


QCString References::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+r_ef );
  else
    return r_ef;
}


void References::fromUnicodeString(const QString &s, const QCString&)
{
  r_ef=s.latin1();
  e_ncCS=cachedCharset(Latin1);
}


QString References::asUnicodeString()
{
  return QString::fromLatin1(r_ef);
}


int References::count()
{
  int cnt1=0, cnt2=0;
  unsigned int r_efLen=r_ef.length();
  char *dataPtr=r_ef.data();
  for(unsigned int i=0; i<r_efLen; i++) {
    if(dataPtr[i]=='<') cnt1++;
    else if(dataPtr[i]=='>') cnt2++;
  }

  if(cnt1<cnt2) return cnt1;
  else return cnt2;
}


QCString References::first()
{
  p_os=-1;
  return next();
}


QCString References::next()
{
  int pos1=0, pos2=0;
  QCString ret;

  if(p_os!=0) {
    pos2=r_ef.findRev('>', p_os);
    p_os=0;
    if(pos2!=-1) {
      pos1=r_ef.findRev('<', pos2);
      if(pos1!=-1) {
        ret=r_ef.mid(pos1, pos2-pos1+1);
        p_os=pos1;
      }
    }
  }
  return ret;
}


QCString References::at(unsigned int i)
{
  QCString ret;
  int pos1=0, pos2=0;
  unsigned int cnt=0;

  while(pos1!=-1 && cnt < i+1) {
    pos2=pos1-1;
    pos1=r_ef.findRev('<', pos2);
    cnt++;
  }

  if(pos1!=-1) {
    pos2=r_ef.find('>', pos1);
    if(pos2!=-1)
      ret=r_ef.mid(pos1, pos2-pos1+1);
  }

 return ret;
}


void References::append(const QCString &s)
{
  QString temp=r_ef.data();
  temp += " ";
  temp += s.data();
  QStringList lst=QStringList::split(' ',temp);
  QRegExp exp("^<.+@.+>$");

  // remove bogus references
  QStringList::Iterator it = lst.begin();
  while (it != lst.end()) {
    if (-1==(*it).find(exp))
      it = lst.remove(it);
    else
      it++;
  }

  if (lst.isEmpty()) {
    r_ef = s.copy();    // shouldn't happen...
    return;
  } else
    r_ef = "";

  temp = lst.first();    // include the first id
  r_ef = temp.latin1();
  lst.remove(temp);         // avoids duplicates
  int insPos = r_ef.length();

  for (int i=1;i<=3;i++) {    // include the last three ids
    if (!lst.isEmpty()) {
      temp = lst.last();
      r_ef.insert(insPos,(QString(" %1").arg(temp)).latin1());
      lst.remove(temp);
    } else
      break;
  }

  while (!lst.isEmpty()) {   // now insert the rest, up to 1000 characters
    temp = lst.last();
    if ((15+r_ef.length()+temp.length())<200) {
      r_ef.insert(insPos,(QString(" %1").arg(temp)).latin1());
      lst.remove(temp);
    } else
      return;
  }
}

//-----</References>---------------------------
#endif


//-----<UserAgent>-----------------------------

void UserAgent::from7BitString(const QCString &s)
{
  u_agent=s;
  e_ncCS=cachedCharset(Latin1);
}


QCString UserAgent::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+u_agent );
  else
    return u_agent;
}


void UserAgent::fromUnicodeString(const QString &s, const QCString&)
{
  u_agent=s.latin1();
  e_ncCS=cachedCharset(Latin1);
}


QString UserAgent::asUnicodeString()
{
  return QString::fromLatin1(u_agent);
}

//-----</UserAgent>----------------------------



#if !defined(KMIME_NEW_STYLE_CLASSTREE)
//-----<Content-Type>--------------------------

void ContentType::from7BitString(const QCString &s)
{
  int pos=s.find(';');

  if(pos==-1)
    m_imeType=s.simplifyWhiteSpace();
  else {
    m_imeType=s.left(pos).simplifyWhiteSpace();
    p_arams=s.mid(pos, s.length()-pos).simplifyWhiteSpace();
  }

  if(isMultipart())
    c_ategory=CCcontainer;
  else
    c_ategory=CCsingle;

  e_ncCS=cachedCharset(Latin1);
}


QCString ContentType::as7BitString(bool incType)
{
  if(incType)
    return (typeIntro()+m_imeType+p_arams);
  else
    return (m_imeType+p_arams);
}


void ContentType::fromUnicodeString(const QString &s, const QCString&)
{
  from7BitString( QCString(s.latin1()) );
}


QString ContentType::asUnicodeString()
{
  return QString::fromLatin1(as7BitString(false));
}


QCString ContentType::mediaType()
{
  int pos=m_imeType.find('/');
  if(pos==-1)
    return m_imeType;
  else
    return m_imeType.left(pos);
}


QCString ContentType::subType()
{
  int pos=m_imeType.find('/');
  if(pos==-1)
    return QCString();
  else
    return m_imeType.mid(pos, m_imeType.length()-pos);
}


void ContentType::setMimeType(const QCString &s)
{
  p_arams.resize(0);
  m_imeType=s;

  if(isMultipart())
    c_ategory=CCcontainer;
  else
    c_ategory=CCsingle;
}


bool ContentType::isMediatype(const char *s)
{
  return ( strncasecmp(m_imeType.data(), s, strlen(s)) );
}


bool ContentType::isSubtype(const char *s)
{
  char *c=strchr(m_imeType.data(), '/');

  if( (c==0) || (*(c+1)=='\0') )
    return false;
  else
    return ( strcasecmp(c+1, s)==0 );
}


bool ContentType::isText()
{
  return (strncasecmp(m_imeType.data(), "text", 4)==0);
}


bool ContentType::isPlainText()
{
  return (strcasecmp(m_imeType.data(), "text/plain")==0);
}


bool ContentType::isHTMLText()
{
  return (strcasecmp(m_imeType.data(), "text/html")==0);
}


bool ContentType::isImage()
{
  return (strncasecmp(m_imeType.data(), "image", 5)==0);
}


bool ContentType::isMultipart()
{
  return (strncasecmp(m_imeType.data(), "multipart", 9)==0);
}


bool ContentType::isPartial()
{
  return (strcasecmp(m_imeType.data(), "message/partial")==0);
}


QCString ContentType::charset()
{
  QCString ret=getParameter("charset");
  if( ret.isEmpty() || forceCS() ) { //we return the default-charset if necessary
    ret=defaultCS();
  }
  return ret;
}


void ContentType::setCharset(const QCString &s)
{
  setParameter("charset", s);
}


QCString ContentType::boundary()
{
  return getParameter("boundary");
}


void ContentType::setBoundary(const QCString &s)
{
  setParameter("boundary", s, true);
}


QString ContentType::name()
{
  const char *dummy=0;
  return ( decodeRFC2047String(getParameter("name"), &dummy, defaultCS(), forceCS()) );
}


void ContentType::setName(const QString &s, const QCString &cs)
{
  e_ncCS=cs;

  if (isUsAscii(s)) {
    QCString tmp(s.latin1());
    addQuotes(tmp, true);
    setParameter("name", tmp, false);
  } else {
    // FIXME: encoded words can't be enclosed in quotes!!
    setParameter("name", encodeRFC2047String(s, cs), true);
  }
}


QCString ContentType::id()
{
  return (getParameter("id"));
}


void ContentType::setId(const QCString &s)
{
  setParameter("id", s, true);
}


int ContentType::partialNumber()
{
  QCString p=getParameter("number");
  if(!p.isEmpty())
    return p.toInt();
  else
    return -1;
}


int ContentType::partialCount()
{
  QCString p=getParameter("total");
  if(!p.isEmpty())
    return p.toInt();
  else
    return -1;
}


void ContentType::setPartialParams(int total, int number)
{
  QCString num;
  num.setNum(number);
  setParameter("number", num);
  num.setNum(total);
  setParameter("total", num);
}


QCString ContentType::getParameter(const char *name)
{
  QCString ret;
  int pos1=0, pos2=0;
  pos1=p_arams.find(name, 0, false);
  if(pos1!=-1) {
    if( (pos2=p_arams.find(';', pos1))==-1 )
      pos2=p_arams.length();
    pos1+=strlen(name)+1;
    ret=p_arams.mid(pos1, pos2-pos1);
    removeQuots(ret);
  }
  return ret;
}


void ContentType::setParameter(const QCString &name, const QCString &value, bool doubleQuotes)
{
  int pos1=0, pos2=0;
  QCString param;

  if(doubleQuotes)
    param=name+"=\""+value+"\"";
  else
    param=name+"="+value;

  pos1=p_arams.find(name, 0, false);
  if(pos1==-1) {
    p_arams+="; "+param;
  }
  else {
    pos2=p_arams.find(';', pos1);
    if(pos2==-1)
      pos2=p_arams.length();
    p_arams.remove(pos1, pos2-pos1);
    p_arams.insert(pos1, param);
  }
}

//-----</Content-Type>-------------------------



//-----<CTEncoding>----------------------------

typedef struct { const char *s; int e; } encTableType;

static const encTableType encTable[] = {  { "7Bit", CE7Bit },
                                          { "8Bit", CE8Bit },
                                          { "quoted-printable", CEquPr },
                                          { "base64", CEbase64 },
                                          { "x-uuencode", CEuuenc },
                                          { "binary", CEbinary },
                                          { 0, 0} };


void CTEncoding::from7BitString(const QCString &s)
{
  QCString stripped(s.simplifyWhiteSpace());
  c_te=CE7Bit;
  for(int i=0; encTable[i].s!=0; i++)
    if(strcasecmp(stripped.data(), encTable[i].s)==0) {
      c_te=(contentEncoding)encTable[i].e;
      break;
    }
  d_ecoded=( c_te==CE7Bit || c_te==CE8Bit );

  e_ncCS=cachedCharset(Latin1);
}


QCString CTEncoding::as7BitString(bool incType)
{
  QCString str;
  for(int i=0; encTable[i].s!=0; i++)
    if(c_te==encTable[i].e) {
      str=encTable[i].s;
      break;
    }

  if(incType)
    return ( typeIntro()+str );
  else
    return str;
}


void CTEncoding::fromUnicodeString(const QString &s, const QCString&)
{
  from7BitString( QCString(s.latin1()) );
}


QString CTEncoding::asUnicodeString()
{
  return QString::fromLatin1(as7BitString(false));
}

//-----</CTEncoding>---------------------------



//-----<CDisposition>--------------------------

void CDisposition::from7BitString(const QCString &s)
{
  if(strncasecmp(s.data(), "attachment", 10)==0)
    d_isp=CDattachment;
  else d_isp=CDinline;

  int pos=s.find("filename=", 0, false);
  QCString fn;
  if(pos>-1) {
    pos+=9;
    fn=s.mid(pos, s.length()-pos);
    removeQuots(fn);
    f_ilename=decodeRFC2047String(fn, &e_ncCS, defaultCS(), forceCS());
  }
}


QCString CDisposition::as7BitString(bool incType)
{
  QCString ret;
  if(d_isp==CDattachment)
    ret="attachment";
  else
    ret="inline";

  if(!f_ilename.isEmpty()) {
    if (isUsAscii(f_ilename)) {
      QCString tmp(f_ilename.latin1());
      addQuotes(tmp, true);
      ret+="; filename="+tmp;
    } else {
      // FIXME: encoded words can't be enclosed in quotes!!
      ret+="; filename=\""+encodeRFC2047String(f_ilename, e_ncCS)+"\"";
    }
  }

  if(incType)
    return ( typeIntro()+ret );
  else
    return ret;
}


void CDisposition::fromUnicodeString(const QString &s, const QCString &cs)
{
  if(strncasecmp(s.latin1(), "attachment", 10)==0)
    d_isp=CDattachment;
  else d_isp=CDinline;

  int pos=s.find("filename=", 0, false);
  if(pos>-1) {
    pos+=9;
    f_ilename=s.mid(pos, s.length()-pos);
    removeQuots(f_ilename);
  }

  e_ncCS=cachedCharset(cs);
}


QString CDisposition::asUnicodeString()
{
  QString ret;
  if(d_isp==CDattachment)
    ret="attachment";
  else
    ret="inline";

  if(!f_ilename.isEmpty())
    ret+="; filename=\""+f_ilename+"\"";

  return ret;
}

//-----</CDisposition>-------------------------
#endif
}; // namespace Headers

}; // namespace KMime
