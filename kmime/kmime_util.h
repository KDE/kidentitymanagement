/*
    kmime_util.h

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
#ifndef __KMIME_UTIL_H__
#define __KMIME_UTIL_H__

#include "qstring.h"
#include "qcstring.h"
#include "qvaluelist.h"

typedef QValueList<QCString> QCStringList;

namespace KMime {

  /** Consult the charset cache. Only used for */
  extern const char* cachedCharset(const QCString &name);

  /** checks whether @p s contains any non-us-ascii characters */
  extern bool isUsAscii(const QString &s);

  inline bool isOfSet(uchar map[16], unsigned char ch) {
#ifndef Q_ASSERT
    ASSERT( ch < 127 );
#else
    Q_ASSERT( ch < 127 );
#endif
    return ( map[ ch/8 ] & 0x80 >> ch%8 );
  }

  extern uchar specialsMap[16];
  extern uchar tSpecialsMap[16];
  extern uchar aTextMap[16];
  extern uchar tTextMap[16];

  inline bool isSpecial(char ch) {
    return isOfSet( specialsMap, ch );
  }
  inline bool isTSpecial(char ch) {
    return isOfSet( tSpecialsMap, ch );
  }
  inline bool isAText(char ch) {
    return isOfSet( aTextMap, ch );
  }
  inline bool isTText(char ch) {
    return isOfSet( tTextMap, ch );
  }

  /** Decode string @p src according to RFC2047 (ie. the
      =?charset?[qb]?encoded?= construct).
      @param src       source string.
      @param usedCS    the detected charset is returned here
      @param defaultCS the charset to use in case the detected
                       one isn't known to us.
      @param forceCS   force the use of the default charset.
      @return the decoded string.
  */
  extern QString decodeRFC2047String(const QCString &src, const char **usedCS,
				     const QCString &defaultCS, bool forceCS);

  /** Encode string @p src according to RFC2047 using charset
      @p charset.
      @param src           source string.
      @param charset       charset to use.
      @param addressheader if this flag is true, all special chars
                           like <,>,[,],... will be encoded, too.
      @param allow8BitHeaders if this flag is true, 8Bit headers
                           are allowed.
      @return the encoded string.
  */
  extern QCString encodeRFC2047String(const QString &src, const char *charset,
				      bool addressHeader=false, bool allow8bitHeaders=false);

  /** Uses current time, pid and random numbers to construct a string
      that aims to be unique on a per-host basis (ie. for the local
      part of a message-id or for multipart boundaries.
      @return the unique string.
      @see multiPartBoundary
  */
  extern QCString uniqueString();

  /** Constructs a random string (sans leading/trailing "--") that can
      be used as a multipart delimiter (ie. as @p boundary parameter
      to a multipart/... content-type).
      @return the randomized string.
      @see uniqueString
  */
  extern QCString multiPartBoundary();

  /** Tries to extract the header with name @p name from the string
      @p src, unfolding it if necessary.
      @param src  the source string.
      @param name the name of the header to search for.
      @return the first instance of the header @p name in @p src
              or a null QCString if no such header was found.
  */
  extern QCString extractHeader(const QCString &src, const char *name);
  /** Converts all occurences of "\r\n" (CRLF) in @p s to "\n" (LF).
      
      This function is expensive and should be used only if the mail
      will be stored locally. All decode functions can cope with both
      line endings.
      @param s source string containing CRLF's
      @return the string with CRLF's substitued for LF's
      @see CRLFtoLF(const char*) LFtoCRLF
  */
  extern QCString CRLFtoLF(const QCString &s);
  /** Converts all occurences of "\r\n" (CRLF) in @p s to "\n" (LF).
      
      This function is expensive and should be used only if the mail
      will be stored locally. All decode functions can cope with both
      line endings.
      @param s source string containing CRLF's
      @return the string with CRLF's substitued for LF's
      @see CRLFtoLF(const QCString&) LFtoCRLF
  */
  extern QCString CRLFtoLF(const char *s);
  /** Converts all occurences of "\n" (LF) in @p s to "\r\n" (CRLF).
      
      This function is expensive and should be used only if the mail
      will be transmitted as an RFC822 message later. All decode
      functions can cope with and all encode functions can optionally
      produce both line endings, which is much faster.

      @param s source string containing CRLF's
      @return the string with CRLF's substitued for LF's
      @see CRLFtoLF(const QCString&) LFtoCRLF
  */
  extern QCString LFtoCRLF(const QCString &s);

  /** Removes quote (DQUOTE) characters and decodes "quoted-pairs"
      (ie. backslash-escaped characters)
      @param str the string to work on.
      @see addQuotes
  */
  extern void removeQuots(QCString &str);
  /** Removes quote (DQUOTE) characters and decodes "quoted-pairs"
      (ie. backslash-escaped characters)
      @param str the string to work on.
      @see addQuotes
  */
  extern void removeQuots(QString &str);
  /** Converts the given string into a quoted-string if
      the string contains any special characters
      (ie. one of ()<>@,.;:[]=\").
      @param str us-ascii string to work on.
      @param forceQuotes if @p true, always add quote characters.
  */
  extern void addQuotes(QCString &str, bool forceQuotes);




} // namespace KMime

#endif /* __KMIME_UTIL_H__ */
