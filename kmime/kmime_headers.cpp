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


#include <qtextcodec.h>

#ifndef Q_ASSERT
#  ifdef ASSERT
#    define Q_ASSERT ASSERT
#  else
#    include <assert.h>
#    define Q_ASSERT assert
#  endif
#endif

#include <kglobal.h>
#include <kcharsets.h>
#include <krfcdate.h>

#include "kmime_content.h"
#include "kmime_headers.h"
#include "kqcstringsplitter.h"

#ifndef KMIME_NO_WARNING
#  include <kdebug.h>
#  define KMIME_WARN kdDebug(5100) << "Tokenizer Warning: "
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

// parse the encoded-word (pos points to after the initial '=?')
QString Base::parseEncodedWord( const QCString & src, int & pos, bool & ok ) {

  // according to IANA, the maximum length for a charset name is 40
  // chars. However, there already is a >40 char charset in the
  // registry, but it's very, very, very unlikely that that will ever
  // be used (and even much more unlikely that there will be a
  // QTextCodec for it :-). So we stick to the IANA limit of 40 chars:
  const int maxCharsetNameLength = 40;
  QCString charset( maxCharsetNameLength + 1);
  int seenCharsetChars = 0;

  // rfc2047 mandates that encoded-words be no longer that 76 chars,
  // which leaves around 65 chars for the encoded-text. But we leave a
  // bit of room for broken mailers.
  const int maxEncodedTextLength = 127;

  // make sure that the caller found a =? pair.
  Q_ASSERT( pos < 2 || src[pos-2] == '=' && src[pos-1] == '?' );

  //
  // scan for the charset portion of the encoded word
  //

  char ch = src[pos++];

  while ( true ) {
    for ( ; ch < 0 && isTText(ch) && seenCharsetChars < maxCharsetNameLength ;
	  ch = src[pos++] ) {
      charset[ seenCharsetChars++ ] = ch;
    }

    // non-tText encountered
    if ( !ch ) {
      ok = false;
      KMIME_WARN_PREMATURE_END_OF(EncodedWord);
      return QString::null;
    }
    if ( seenCharsetChars > maxCharsetNameLength ) {
      ok = false;
      KMIME_WARN_TOO_LONG(CharsetName);
      return QString::null;
    }
    if ( ch == '?' || ch == '*' ) {
      // found charset specifier end marker
      // or language specifier intro (rfc 2231)
      charset.truncate( seenCharsetChars );
      break;
    }
    // any other non-ttext char, replace with '_':
    if ( ch < 0 ) {
      KMIME_WARN_8BIT(ch);
    } else {
      //      KMIME_WARN_INVALID_X_IN_Y(ch,charset_specifier);
    }
    charset[ seenCharsetChars++ ] = '_';
  } // while true

  //
  // parse language tag, if any
  //

  if ( ch == '*' ) {
    ch = src[pos++];

    int seenLangChars = 0;
    // take only primary and secondary into account.
    const int maxLangNameLength = 2*8 /* primary, secondary */ + 1 /* dash */;

    QCString language( maxLangNameLength + 1 );
    // have to parse language tag:
    while ( true ) {
      for ( ; ch < 0 && isTText(ch) && seenLangChars < maxLangNameLength ;
	    ch = src[pos++] ) {
	language[ seenLangChars++ ] = ch;
      }

      // non-tText encountered
      if ( !ch ) {
	ok = false;
	KMIME_WARN_PREMATURE_END_OF(EncodedWord);
	return QString::null;
      }
      if ( seenLangChars > maxLangNameLength ) {
	ok = false;
	KMIME_WARN_TOO_LONG(LanguageName);
	return QString::null;
      }
      if ( ch == '?' ) {
	// found end marker
	language.truncate( seenLangChars );
	break;
      }
      // any other non-ttext char, replace with '_':
      if ( ch < 0 ) {
	KMIME_WARN_8BIT(ch);
      } else {
	//      KMIME_WARN_INVALID_X_IN_Y(ch,language_tag);
      }
      language[ seenLangChars++ ] = '_';
    }
  }

  //
  // find suitable QTextCodec
  //

  QTextCodec * codec = 0;
  bool matchOK = true;

  // try a match.
  codec = KGlobal::charsets()->codecForName( charset, matchOK );

  if (!matchOK) {
    KMIME_WARN_UNKNOWN_CHARSET(charset);
  }

  //
  // scan for the encoding type
  //

  char enc = src[pos++];

  if ( !enc ) {
    ok = false;
    KMIME_WARN_PREMATURE_END_OF(EncodedWord);
    return QString::null;
  }

  //
  // branch off according to encoding found
  //

  ch = src[pos++];

  QCString eightBit( maxEncodedTextLength + 1 );
  int eightBitCursor = 0;

  if ( ( enc == 'Q' || enc == 'q' ) && ch == '?' ) {
    // quoted-printable encoding, rfc2047 variant; correct charset and
    // encoding found, so we report the encoded-word as correct, even
    // though we might see later on that it's broken in the
    // encoded-text part.

    // remember the pos of the first white space, so we can use it as
    // an delimiter in case we run till the end of the string w/o
    // seeing '?='
    int afterFirstWsp = 0;
    int afterFirstWspEightBitCursor = -1;

    ch = src[pos++];

    while( ch && ch != '?' && eightBitCursor < maxEncodedTextLength ) {
      switch ( ch ) {
      case '=': // maybe hex-char
	{
	  uchar accu = 0;
	  // first nibble:
	  ch = src[pos++];
	  if ( !ch ) {
	    KMIME_WARN_PREMATURE_END_OF(EncodedWord);
	    break; // will break out of while loop since ch == 0
	  } else if ( ch >= '0' && ch <= '9' ) {
	    accu = (ch - '0') << 4;
	  } else if ( ch >= 'A' && ch <= 'F' ) {
	    accu = (ch - 'A' + 10) << 4;
	  } else if ( ch >= 'a' && ch <= 'f' ) {
	    // not allowed by rfc2047/45, but nonetheless accepted
	    //	    KMIME_WARN_INVALID_X_IN_Y(ch,HexChar);
	    accu = (ch - 'a' + 10) << 4;
	  } else {
	    //	    KMIME_WARN_INVALID_X_IN_Y(ch,HexChar);
	    eightBit[ eightBitCursor++ ] = '=';
	    break; // will break out of while loop if ch == '?'
	  }
	  // second nibble:
	  ch = src[pos++];
	  if ( !ch ) {
	    //	    KMIME_WARN_PREMATURE_END_OF(HexChar);
	    break; // will break out of while loop since ch == 0
	  } else if ( ch >= '0' && ch <= '9' ) {
	    accu |= (ch - '0') & 0x0F;
	  } else if ( ch >= 'A' && ch <= 'F' ) {
	    accu |= (ch - 'A' + 10) & 0x0F;
	  } else if ( ch >= 'a' && ch <= 'f' ) {
	    // not allowed by rfc2047/45, but nonetheless accepted
	    //	    KMIME_WARN_INVALID_X_IN_Y(ch,HexChar);
	    accu |= (ch - 'a' + 10) & 0x0F;
	  } else {
	    //	    KMIME_WARN_INVALID_X_IN_Y(ch,HexChar);
	    eightBit[ eightBitCursor++ ] = '=';
	    eightBit[ eightBitCursor++ ] = src[pos-2];
	    pos--; // position cursor over current "ch" again
	    break; // will break out of while loop if ch == '?'
	  }
	  // if control reaches here, we have found a valid hex-char
	  eightBit[ eightBitCursor++ ] = (char)accu;
	  ch = src[pos++];
	}
	break;

      case '_': // shortcut-encoding of '=20'
	eightBit[ eightBitCursor++ ] = 0x20;
	ch = src[pos++];
	break;

      case ' ':
      case '\t':
      case '\n':
      case '\r':
	// white space is not allowed, but we accept it if there's a
	// valid '?=' sequence later on...
	if (!afterFirstWsp) {
	  afterFirstWsp = pos;
	  afterFirstWspEightBitCursor = eightBitCursor;
	}
	eightBit[ eightBitCursor++ ] = ch;
	ch = src[pos++];
	break;

      default:
	if ( ch < 127 ) {
	  KMIME_WARN_8BIT(ch);
	  eightBit[ eightBitCursor++ ] = ch;
	  ch = src[pos++];
	  break;
	}
	if ( !isAText(ch) ) {
	  //	  KMIME_WARN_INVALID_X_IN_Y(ch,EncodedText);
	  if ( (unsigned char)ch > 32 && ch != 127 )
	    // only add printable non-atext
	    eightBit[ eightBitCursor++ ] = ch;
	} else {
	  eightBit[ eightBitCursor++ ] = ch;
	}
	ch = src[pos++];
	break;	
      } // switch
    } // while

    // check while loop abort condition
    if ( !ch ) {
      // premature end. simply ignore this fact.
      KMIME_WARN_PREMATURE_END_OF(EncodedText);
    } else if ( ch == '?' ) {
      // we take '?' to mean the end of encoded-text, but we should
      // probably just include it in encoded-text, at least if there
      // is a '?=' later on.
    }
    eightBit.truncate( eightBitCursor );

  } else if ( ( enc == 'B' || enc == 'b' ) && ch == '?' ) {
    // base64 encoding
    eightBit = decodeBase64( src, pos, "?" /* end marker */ );
  } else if ( !ch ) {
    ok = false;
    KMIME_WARN_PREMATURE_END_OF(EncodedWord);
    return QString::null;
  } else {
    ok = false;
    KMIME_WARN_UNKNOWN_ENCODING;
    return QString::null;
  }

  //
  // if we reach here, we have seen at least the encoded-word
  // "header". The encoded-text might have been corrupted, broken or
  // truncated, but at least we have something to return...
  //

  QString result = codec->toUnicode( eightBit );

  ok = true;
  return result;

}


//-----</Base>---------------------------------



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

// known issues:
//
// - EncodedWords parsing not yet implemented
// - EncodedWords decoding in Comments not handled
// - 

QString GStructured::getToken( const QCString & source, int & pos,
			       TokenType & tt, bool isCRLF )
{
  // pos must be in range, but may point to the termiating NUL
  Q_ASSERT( pos >= 0 && pos <= (int)source.length() );
  // if caller disallows composite types, he must allow the constituends:
#if 0
  Q_ASSERT( tt & Atom          || tt & Token && tt & TSpecial );
  Q_ASSERT( tt & Special       || tt & TSpecial );
  Q_ASSERT( tt & EncodedWord   || tt & Atom || tt & Token && tt & TSpecial );
  Q_ASSERT( tt & (QuotedString|DomainLiteral)
	                       || tt & Atom && tt & (TSpecial|Special)
	                       || tt & Token && tt & TSpecial );
  Q_ASSERT( tt & DotAtom       || tt & Atom && tt & (Special|TSpecial) );
  Q_ASSERT( tt & Phrase        || tt & EncodedWord && tt & Atom
	                            && tt & QuotedString && tt & Token );
#endif
  TokenType found = None;
  bool lastSawDotInDotAtom = false;
  bool lastWasEncodedWord = false;
  int oldpos;
  char ch;
  QString result;

  while ( (ch = source[pos++]) ) {

    switch ( ch ) {

      /* ============================================== */

    case ' ':
    case '\t':  // skip whitespace
      //      kdDebug() << "skipping whitespace at pos " << pos-1 << endl;
      break;
    
      /* ============================================== */

    case '\r':      // possible folding
      //      kdDebug() << "CR: possible folding" << endl;
      ch = source[pos++];
      if ( !ch ) {
	tt = found; pos--; // set cursor to point to NUL
	return result;
      }
      if ( ch != '\n' ) {
	// CR on it's own: forbidden outside generic quoted strings.
	ch = source[pos++];
	if ( ch == ' ' || ch == '\t' ) {
	  // looks like a broken folded header:
	  // we take it to mean folding and simply ignore it.
	  KMIME_WARN_CTL_OUTSIDE_QS(CR);
	} else if ( ch == '\0' ) {
	  // ### end of header.
	  KMIME_WARN_CTL_OUTSIDE_QS(CR);
	  tt = found; pos--;
	  return result;
	} else {
	  // non-WSP following \r: We ignore the \r, too, but leave
	  // the processing of "ch" to the next iteration
	  KMIME_WARN_NON_FOLDING(CR);
	  pos--;
	}
	break;
      }
      //
      // when control reaches here, we found a CRLF, so unfold it:
      //
      ch = source[pos++];
      if ( ch == '\0' ) {
	// ### end of header
	tt = found; pos--;
	return result;
      } else if ( ch != ' ' && ch != '\t' ) {
	// non-WSP after CRLF is forbidden. We choose to ignore the
	// CRLF nontheless.
	KMIME_WARN_NON_FOLDING(CRLF);
	pos--; // let the next iteration handle "ch"
      }
      break;

      /* ============================================== */

    case '\n':
      //      kdDebug() << "LF: possible folding" << endl;
      // for a discussion of what we do here see the comment in
      // parseGenericQuotedString, case '\n', below.
      ch = source[pos++];
      if ( !isCRLF && ( ch == ' ' || ch == '\t' || ch == '\0' ) ) {
	// folding
	if ( ch == '\0' ) {
	  // ### end of header
	  tt = found; pos--;
	  return result;
	}
	// correct folding, ignore LF WSP
      } else {
	KMIME_WARN_NON_FOLDING(LF);
	pos--;
      }
      break;

      /* ============================================== */

    case '"': // introducing a quoted-string
      //      kdDebug() << "\": maybe quoted-string" << endl;
      if ( ! (tt & QuotedString) ) {
	// no quoted-string allowed
	if ( found != None ) {
	  // already found something, return it and set the cursor so
	  // that it points to '"' again:
	  tt = found; pos--;
	  return result;
	} else {
	  // nothing found up till now, so return the '"' as as
	  // special:
	  tt = (tt & Special) ? Special : TSpecial;
	  result += QChar('"');
	  return result;
	}
      } else {
	//
	// QuotedString is allowed.
	//
	//	kdDebug() << "quoted-string is allowed" << endl;
	if ( tt & Phrase ) {
	  // We are searching for a phrase (among other things)
	  if ( found != None && !(found & (Word|EncodedWord|Phrase)) ) {
	    // what we found until now is INcompatible with a
	    // phrase. Return what we have and wait for the next
	    // invokation:
	    tt = found; pos--;
	    return result;
	  }
	  // continue processing below....
	  //	  kdDebug() << "allowed to return Phrase, and only compat. tokens found yet" << endl;
	} else {
	  // Phrase is not allowed
	  if ( found != None ) {
	    // already found something, return it and set the cursor
	    // so that it points to '"' again:
	    tt = found; pos--;
	    return result;
	  }
	  //	  kdDebug() << "Phrase is not allowed, but we found nothing up till now" << endl;
	  // continue processing below....
	}
	//
	// if control reaches here, we are in one of two cases:
	//
	// 1. We have nothing up till now (found == None), so we try
	// to parse the quoted string and remember it or the '"' as
	// special if we fail.
	// 
	// 2. We have a Word or Phrase already, so we append the
	// quoted string (if parsing is successful), or return what we
	// have and leave the '"' handling as special to the next
	// invokation
	//
	oldpos = pos;
	QString qs = parseGenericQuotedString( source, pos, '"', '"' );
	Q_ASSERT( source[pos-1] == '"' || source[pos-1] == '\0' );
	// see the kdoc for above function for the possible conditions
	// we have to check:
	switch ( source[pos-1] ) {
	case '"':  // normal end of quoted-string
	  if ( found == None ) {
	    found = QuotedString;  // case (1) above
	  } else {
	    found = Phrase;        // case (2) above
	    lastWasEncodedWord = false;
	    result += ' ';         // rfc822, 3.4.4
	  }
	  result += qs;
	  //	KMIME_WARN_IF_INCORRECTLY_DELIMITED(QuotedString,source[pos]);
	  break;
	default:
	case '\0': // premature end of quoted-string.
	  if ( found == None ) {   // case (1) above
	    tt = (tt & Special) ? Special : TSpecial;
	    pos = oldpos;
	    result += QChar('"');
	    return result;
	  } else {                 // case (2) above
	    // can't return Special yet.
	    tt = found; pos = oldpos - 1;
	    return result;
	  }
	}
      }
      break;

      /* ============================================== */

    case '(': // introducing a comment
      //      kdDebug() << "(: maybe comment" << endl;
      if ( found != None && (tt & Comment) ) {
	// we are asked to return comments, but comments can't be
	// aggregated (e.g. into a Phrase), so we return what we have:
	tt = found; pos--;
	return result;
      } else {
	//
	// if contol reaches here, we have either nothing found yet or
	// else are going to ignore this commnet anyway.
	//
	//
	// Comments may nest. In case of unbalanced parentheses, this
	// parser assumes the last ')' to signify the end of the
	// comment. This behaviour is not canonical, so it is to be seen
	// if the other approach (ignoring the rest of the header) may
	// not be better.
	//
	int commentNestingDepth = 1;
	int afterLastClosingParenPos = 0;
	QString cmnt(QChar('('));
	QString maybeCmnt;
	oldpos = pos;
	
	while ( commentNestingDepth ) {
	  QString cmntPart = parseGenericQuotedString( source, pos, '(', ')' );
	  Q_ASSERT( source[pos-1] == ')' || source[pos-1] == '(' || source[pos-1] == '\0' );
	  // see the kdoc for above function for the possible conditions
	  // we have to check:
	  switch ( source[pos-1] ) {
	  case ')':
	    if ( tt & Comment ) {
	      // add the chunk that's now surely inside the comment.
	      cmnt += maybeCmnt;
	      cmnt += cmntPart;
	      cmnt += QChar(')');
	      maybeCmnt = QString::null;
	    }
	    afterLastClosingParenPos = pos;
	    --commentNestingDepth;
	    break;
	  case '(':
	    if ( tt & Comment ) {
	      // don't add to "cmnt" yet, because we might find that we
	      // are already outside the (broken) comment...
	      maybeCmnt += cmntPart;
	      maybeCmnt += QChar('(');
	    }
	    ++commentNestingDepth;
	    break;
	  default:
	  case '\0':
	    // OK, premature end. Now the fun part begins...
	    // (parseGenericQuotedString has already warned)
	    if ( afterLastClosingParenPos ) {
	      // we've seen at least one ')'
	      pos = afterLastClosingParenPos;
	      if ( tt & Comment ) {
		tt = Comment;
		return cmnt;
	      } else {
		// forget the comment and go on.
		goto CONTINUE_OUTER_LOOP;
	      }
	    } else {
	      if ( found == None ) {
		// We haven't seen a single ')', so we return the
		// initial '(' as a Special, regardless of whether we
		// were asked for Comments, too, or not.
		tt = (tt & Special) ? Special : TSpecial;
		pos = oldpos;
		result += QChar('(');
		return result;
	      } else {
		// we've already had something, so we can't return the
		// '(' as Special yet.
		tt = found; pos = oldpos - 1;
		return result;
	      }
	    }
	  } // switch
	} // while
	//
	// if control reaches here, we have seen a valid (ie. balanced)
	// comment. Return it if asked for, else forget it.
	//
	// ### handle endoded-words in comments.
	if ( tt & Comment ) {
	  tt = Comment;
	  return cmnt;
	}
      }
      break;

      /* ============================================== */

    case '[': // introducing a domain-literal
      //      kdDebug() << "[: maybe domain-literal" << endl;
      if ( found != None ) {
	// domain-literal's can't be aggregated
	tt = found; pos--;
	return result;
      }

      if ( ! (tt & DomainLiteral) ) {
	// we're not allowed to return a domain-literal, so we return
	// it's '[' as a Special
	tt = (tt & Special) ? Special : TSpecial;
	result += QChar('[');
	return result;
      } else {

	//
	// if control reaches here, we are allowed to parse the
	// domain-literal and will return it as such if we succeed in
	// parsing, or it's first '[' as Special if we don't.
	//
	
	QString dl;
	int oldpos = pos;
	
	while (true) {
	  dl += parseGenericQuotedString( source, pos, '[', ']' );
	  Q_ASSERT( source[pos-1] == ']' || source[pos-1] == '[' || source[pos-1] == '\0' );
	  // see the kdoc for above function for the possible conditions
	  // we have to check:
	  switch ( source[pos-1] ) {
	  case ']': // domain-literal end
	    tt = DomainLiteral;
	    return dl;
	  
	  case '[': // allow in domain-lieral, but warn
	    KMIME_WARN_LONE('[');
	    dl += QChar('[');
	    break;
	    
	  default:
	  case '\0': // premature end.
	    // return the initial '[' as Special and reset the cursor to
	    // after that '['
	    pos = oldpos;
	    tt = (tt & Special) ? Special : TSpecial;
	    result += QChar('[');
	    return result;
	  }
	}
      }
      break;

      /* ============================================== */

    case '.': // continuing a dot-atom
      //      kdDebug() << ".: maybe continuing a dot-atom" << endl;
      if ( tt & DotAtom ) {
	if ( found == None ) {
	  // dot-atom can't begin with '.'! Return as Special or
	  // TSpecial:
	  tt = (tt & Special) ? Special : TSpecial;
	  result += QChar('.');
	  return result;
	} else if ( found & Atom ) {
	  // this is what we want.
	  found = DotAtom;
	  lastSawDotInDotAtom = true;
	  result += QChar('.');
	} else if ( found & DotAtom ) {
	  if ( lastSawDotInDotAtom ) {
	    // no, we can't have '..' in DotAtom's
	    tt = DotAtom; pos--;
	    return result;
	  } else {
	    // this fits, we have a dot following an Atom:
	    lastSawDotInDotAtom = true;
	    result += QChar('.');
	  }
	} else {
	  // INcompatible tokens found, return them first.
	  tt = found; pos--;
	  return result;
	}
      } else {
	// no dot-atom allowed
	if ( found == None ) {
	  tt = ( tt & Special ) ? Special : TSpecial;
	  result += QChar('.');
	  return result;
	} else {
	  // return other found stuff first
	  tt = found; pos--;
	  return result;
	}
      }
      break;

      /* ============================================== */

    case '=': // introducing an encoded word.
      //      kdDebug() << "=: maybe introducing an encoded-word" << endl;
      oldpos = pos;

      if ( found != None &&
	   ( !(tt & Phrase) || !(found & (Word|Phrase|EncodedWord)) ) ) {
	// we're not allowed to return phrases, but we already found
	// something
	// -or-
	// we found something incompatible before:
	// return that first
	tt = found; pos--;
	return result;
      }
      //
      // if control reaches here, we either have compatible tokens in
      // "result" (and are allowed to aggregate them into a Phrase),
      // or nothing.
      //
      if ( tt & EncodedWord ) {
	// allowed to return encoded words
	ch = source[pos++];
	if ( ch == '?' ) {
	  // next char == '?', so we might have a chance to see a
	  // valid encoded-word:
	  bool ok = false;
	  QString ew = parseEncodedWord( source, pos, ok );
	  if ( ok ) {
	    Q_ASSERT( source[pos-1] == '=' || source[pos-1] == '\0' );
	    // ### check what else can happen when parseEncodedWord is
	    // implemented, and insert it into the assertion ....
	    if ( found == None ) {
	      // label our find
	      found = EncodedWord;
	    } else {
	      // found something already (we can be sure that tt &
	      // Phrase == true, since we eliminated all other cases
	      // above, so we assert that:)
	      Q_ASSERT( tt & Phrase && found & (EncodedWord|Word|Phrase) );
	      if ( found != EncodedWord ) {
		// found something compatible to form a Phrase
		// of. We exclude the case of EncodedWord, because
		// we don't re-label consecutive encoded-words, but
		// simply concatenate them under the EncodedWord
		// label
		found = Phrase;
		if ( !lastWasEncodedWord ) {
		  // in case it's a phrase, and the last token
		  // appended to it wasn't an encoded-word, we have
		  // to add a space (rfc822, 3.4.4)
		  result += QChar(' ');
		}
	      }
	    }
	    // always set the bool flag and append the encoded word.
	    lastWasEncodedWord = true;
	    result += ew;
	    break;
	  } // fi ( ok )
	} // fi ( ch == '?' )
	// ch != '?' or encoded-word not ok, so fall through to the
	// token/atom parser...
      }
      //
      // if control reaches here, we're not allowed to return
      // EncodedWord's (or failed to parse it correctly), so we
      // restore the cursor and "ch" and fall through to the
      // token/atom parser:
      // 
      pos = oldpos;
      ch = source[pos-1];

      // fall through

      /* ============================================== */

    default: // token/atom/special/tspecial

      //      kdDebug() << "default branch hit with ch == " << QString(QChar(ch))
      //		<< " at pos == " << pos-1 << endl;
      // introducing tokens or tspecials, which may end up being atoms
      // or specials
      
      // first, collect chars that make up tokens (TText, though
      // rfc2045 doesn't call it like that)

      if ( tt & Atom ) {
	// we may return Atoms, so collect chars that make up atoms
	// (AText, see rfc2822, which agrees on rfc822 on this), but
	// first check whether we need to return something:
	if ( !( found == None ||
		tt & Phrase && found & (Word|EncodedWord|Phrase) ||
		tt & DotAtom && found & DotAtom && lastSawDotInDotAtom ) ) {
	  // inompatible tokens found, return what we have. Note that
	  // the case found == None is already contained in the above
	  // branch.
	  tt = found; pos--;
	  return result;
	}
	//
	// if control reaches here, we have either permission to
	// return Phrases (and have only seen compatible tokens up
	// till now), or DotAtoms (and have only seen DotAtom with
	// lastSawDotInDotAtom), or we've found nothing.
	//
	QString maybeAtom;
	for ( ; true ; ch = source[pos++] ) {
	  // short-cut collect AText
	  for ( ; ch > 0 && isAText(ch) ; ch = source[pos++] )
	    maybeAtom += QChar(ch);
	  //
	  // no AText, so let's see what we got...
	  //
	  //	  kdDebug() << "hit non-atext '" << QString(QChar(ch))
	  //		    << "' in default branch at pos " << pos-1 << endl;
	  if ( !ch ) {
	    // ### end of header
	    //
	    // it shouldn't happen that the first thing we saw (and
	    // what thus could've made maybeAtom be empty) was NUL,
	    // because this should have been catched by the outer loop
	    // earlier on...
	    Q_ASSERT( !maybeAtom.isEmpty() );
	    if ( found == None ) {
	      tt = Atom; pos--;
	      return maybeAtom;
	    } else if ( found == DotAtom ) {
	      Q_ASSERT( lastSawDotInDotAtom );
	      tt = DotAtom; pos--;
	      result += maybeAtom;
	      return result;
	    } else {
	      // already found something. Note, we can be sure that
	      // that is compatible with a phrase, see the beginning
	      // of this case.
	      Q_ASSERT( tt & Phrase );
	      Q_ASSERT( found & (Phrase|Word|EncodedWord) );
	      result += QChar(' '); // rfc822, 3.4.4
	      result += maybeAtom;
	      tt = Phrase; pos--;
	      return result;
	    }
	  } else if ( ch == ' ' || ch == '.' || ch == '(' || ch == '=' ||
		      ch == '\n' || ch == '\r' || ch == '\t' ) {
	    // it shouldn't happen that the first thing we saw (and
	    // what thus could've made maybeAtom be empty) was WSP /
	    // CR / LF / "." / "(", because this should have been
	    // catched by the outer loop earlier on...
	    Q_ASSERT( !maybeAtom.isEmpty() );
	    //
	    // " \n\r\t.(=" also end our Atom, but if we're searching
	    // for a Phrase or DotAtom, other Words or Specials ('.')
	    // may come after this non-atext, so we add our maybeAtom
	    // to the result and break out into the outer loop:
	    if ( found == None ) {
	      found = Atom;
	    } else if ( found == DotAtom ) {
	      Q_ASSERT( lastSawDotInDotAtom );
	      lastSawDotInDotAtom = false;
	    } else {
	      Q_ASSERT( tt & Phrase );
	      Q_ASSERT( found & (Phrase|Word|EncodedWord) );
	      result += QChar(' '); // rfc822, 3.4.4
	      lastWasEncodedWord = false;
	      found = Phrase;
	    }
	    // in any case: add maybeAtom, adjust cursor to point to
	    // "ch" again, and (effectively) break into OUTER_LOOP:
	    result += maybeAtom; pos--;
	    break;
	  } else if ( ch < 0 ) {
	    // seems to be an 8Bit char: warn, but add it nonetheless.
	    KMIME_WARN_8BIT(ch);
	    maybeAtom += QChar(ch);
	  } else if ( isSpecial(ch) ) {
	    // "ch" is a Special (excluding ".", which is handled
	    // above). This definitely ends any Phrase or DotAtom that
	    // we may have found earlier: return what we have, or "ch"
	    // as Special or TSpecial, if there wasn't anything yet:
	    if ( !maybeAtom.isEmpty() ) {
	      if ( found == None ) {
		tt = Atom; pos--;
		return maybeAtom;
	      } else if ( found == DotAtom ) {
		Q_ASSERT( lastSawDotInDotAtom );
		result += maybeAtom;
		tt = DotAtom; pos--;
		return result;
	      } else {
		Q_ASSERT( tt & Phrase );
		Q_ASSERT( found & (Phrase|Word|EncodedWord) );
		result += QChar(' '); // rfc822, 3.4.4
		result += maybeAtom;
		tt = Phrase; pos--;
		return result;
	      }
	    } else {
	      // maybeAtom is empty
	      if ( found == None ) {
		tt = (tt & Special) ? Special : TSpecial;
		result += QChar(ch);
		return result;
	      } else {
		// already found something (either DotAtom or part of
		// Phrase):
		Q_ASSERT( found & (DotAtom|Phrase|Word|EncodedWord) );
		tt = found; pos--;
		return result;
	      }
	    }
	  } else {
	    // "ch" is a CTL (excluding CR and LF):
	    Q_ASSERT( (unsigned char)ch < 32 || ch == 127 );
	    // We warn and simply ignore it.
	    KMIME_WARN_CTL_OUTSIDE_QS(ch);
	  }
	} // for true
      } else {
	// we may not return Atoms, but then we must have been allowed
	// to return Tokens and TSpecials, of which Atoms consist.
	Q_ASSERT( tt & Token && tt & TSpecial );

	// Note that we don't need to consider Phrases here, since
	// they mandate that Atoms be allowed, and thus we wouldn't
	// end up here in the first place.
	if ( found != None ) {
	  // found something already: return that first.
	  tt = found; pos--;
	  return result;
	}

	QString maybeToken;
	for ( ; true ; ch = source[pos++] ) {
	  // short-cut collect tText:
	  for ( ; ch > 0 && isTText(ch) ; ch = source[pos++] )
	    maybeToken += QChar(ch);
	  
	  // non-ttext-char encountered.
	  
	  if ( ch == ' ' || ch == '\n' ||
	       ch == '\0' || ch == '\r' || ch == '\n' ) {
	    // NUL, SPACE, HTAB, '(', CR or LF encoutered; see in the (tt &
	    // Atom) branch for why this assert is here
	    Q_ASSERT( !maybeToken.isEmpty() );
	    tt = Token; pos--;
	    return maybeToken;
	  } else if ( ch < 0 ) {
	    // seems to be an 8Bit char: warn, but add it nonetheless.
	    KMIME_WARN_8BIT(ch);
	    maybeToken += QChar(ch);
	  } else if ( isTSpecial(ch) ) {
	    // TSpecials definitely end our Token
	    if ( maybeToken.isEmpty() ) {
	      // no Token found: return the (T)Special
	      if ( tt & Special && ch != '/' && ch != '=' && ch != '?' )
		// allowed to return Special and isSpecial(ch)
		tt = Special;
	      else
		tt = TSpecial;
	      result += QChar(ch);
	      return result;
	    } else {
	      // Token was found
	      tt = Token; pos--;
	      return maybeToken;
	    }
	  } else {
	    // "ch" is a CTL (excluding CR and LF):
	    Q_ASSERT( (unsigned char)ch < 32 || ch == 127 );
	    // We warn and simply ignore it.
	    KMIME_WARN_CTL_OUTSIDE_QS(ch);
	  }
	} // for true
      }
      break;

    } // biiiig switch
  CONTINUE_OUTER_LOOP: ;
  } // while

  // ### end of header (OUTER_LOOP is left on ch == '\0')
  tt = found; pos--;
  return result;

} // getToken






QString GStructured::parseGenericQuotedString( const QCString & src, int & pos,
		      const char openChar, const char closeChar, bool isCRLF )
{
  QString result = "";
  char ch;
  // We are in a quoted-string or domain-literal or comment and the
  // cursor (pos) points to the first char after the openChar.
  // We will apply unfolding and quoted-pair removal.
  // We return when we either encounter \0 or unescaped openChar or closeChar.

  while ( (ch = src[pos++]) ) {

    if ( ch == closeChar     // end of quoted-string
	 || ch == openChar ) // another opening char: let caller decide what to do.
      return result;

    switch( ch ) {
    case '\\':      // quoted-pair
      // misses "\" CRLF LWSP-char handling, see rfc822, 3.4.5
      if ( (ch = src[pos++]) ) {
	// not NULL, \x -> x
	KMIME_WARN_IF_8BIT(ch);
	result += QChar(ch);
      } else {
	// NULL, panic!
	KMIME_WARN_PREMATURE_END_OF(GenericQuotedString);
	return result;
	// ### how should we handle '\0' chars,
	// which are allowed in rfc822, but not 2822?
	// This problem will go away when we use QByteArrays.
      }
      break;
    case '\r':
      // ###
      // The case of lonely '\r' is easy to solve, as they're
      // not part of Unix Line-ending conventions.
      // But I see a problem if we are given Unix-native
      // line-ending-mails, where we cannot determine anymore
      // whether a given '\n' was part of a CRLF or was occuring
      // on it's own.
      ch = src[pos++];
      if ( !ch ) {
	// ### ASCII NULL encountered inside quoted-string!
	// NOTE: here we violate the rfc. We should just skip it.
	// pos points to NULL now
	KMIME_WARN_PREMATURE_END_OF(GenericQuotedString);
	return result;
      }
      if ( ch != '\n' ) {
	// CR on it's own...
	KMIME_WARN_LONE(CR);
	result += QChar('\r');
	pos--; // points to after the '\r' again
      } else {
	// CRLF encountered.
	// lookahead: check for folding
	ch = src[pos++];
	if ( ch == ' ' || ch == '\t' ) {
	  // correct folding;
	  // position cursor behind the CRLF WSP (unfolding)
	  // and add the WSP to the result
	  result += QChar(ch);
	} else if ( !ch ) {
	  // ###
	  // ASCII NULL encountered inside quoted-string!
	  // NOTE: here we violate the rfc. We should just skip it.
	  // pos points to after NULL now
	  KMIME_WARN_PREMATURE_END_OF(GenericQuotedString);
	  return result;
	} else {
	  // this is the "shouldn't happen"-case. There is a CRLF
	  // inside a quoted-string without it being part of FWS.
	  // We take it verbatim.
	  KMIME_WARN_NON_FOLDING(CRLF);
	  result += "\r\n";
	  // "pos" is decremented agian, so's we need not duplicate
	  // the whole switch here. "ch" could've been everything
	  // (incl. <">, "\").
	  pos--;
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
      ch = src[pos++];
      if ( !isCRLF && ( ch == ' ' || ch == '\t' || ch == '\0' ) ) {
	// folding
	if ( ch == '\0' ) {
	  // ### end of header
	  KMIME_WARN_PREMATURE_END_OF(GenericQuotedString);
	  return result;
	}
	// correct folding
	result += QChar(ch);
      } else {
	// non-folding
	KMIME_WARN_LONE(LF);
	result += QChar('\n');
	// pos is decremented, so's we need not duplicate the whole
	// switch here. ch could've been everything (incl. <">, "\").
	pos--;
      }
      break;
    default:
      KMIME_WARN_IF_8BIT(ch);
      result += QChar(ch);
    }
  }

  return result;
}

//-----</GStructured>-------------------------




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

}; // namespace Headers

}; // namespace KMime
