/*
    kmime_util.cpp

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
#include "kmime_util.h"

#include <kmdcodec.h> // for KCodec::{quotedPrintableDe,base64{En,De}}code
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>

#include <qtextcodec.h>
#include <qstrlist.h> // for QStrIList
#include <qregexp.h>

#include <stdlib.h>
#include <ctype.h>
#include <time.h> // for time()
#include <unistd.h> // for getpid()

using namespace KMime;

namespace KMime {

QStrIList c_harsetCache;

const char* cachedCharset(const QCString &name)
{
  int idx=c_harsetCache.find(name.data());
  if(idx>-1)
    return c_harsetCache.at(idx);

  c_harsetCache.append(name.upper().data());
  //qDebug("KNMimeBase::cachedCharset() number of cs %d", c_harsetCache.count());
  return c_harsetCache.last();
}

bool isUsAscii(const QString &s)
{
  for (uint i=0; i<s.length(); i++)
    if (s.at(i).latin1()<=0)    // c==0: non-latin1, c<0: non-us-ascii
      return false;

  return true;
}

// "(),.:;<>@[\]
uchar specialsMap[16] = {
  0x00, 0x00, 0x00, 0x00, // CTLs
  0x20, 0xCA, 0x00, 0x3A, // SPACE ... '?'
  0x80, 0x00, 0x00, 0x1C, // '@' ... '_'
  0x00, 0x00, 0x00, 0x00  // '`' ... DEL
};

// "(),.:;<>@[\]/=?
uchar tSpecialsMap[16] = {
  0x00, 0x00, 0x00, 0x00, // CTLs
  0x20, 0xCB, 0x00, 0x3F, // SPACE ... '?'
  0x80, 0x00, 0x00, 0x1C, // '@' ... '_'
  0x00, 0x00, 0x00, 0x00  // '`' ... DEL
};

// all except specials, CTLs, SPACE.
uchar aTextMap[16] = {
  0x00, 0x00, 0x00, 0x00,
  0x5F, 0x35, 0xFF, 0xC5,
  0x7F, 0xFF, 0xFF, 0xE3,
  0xFF, 0xFF, 0xFF, 0xFE
};

// all except tspecials, CTLs, SPACE.
uchar tTextMap[16] = {
  0x00, 0x00, 0x00, 0x00,
  0x5F, 0x34, 0xFF, 0xC0,
  0x7F, 0xFF, 0xFF, 0xE3,
  0xFF, 0xFF, 0xFF, 0xFE
};

QString decodeRFC2047String(const QCString &src, const char **usedCS,
			    const QCString &defaultCS, bool forceCS)
{
  QCString result, str;
  QCString declaredCS;
  char *pos, *dest, *beg, *end, *mid, *endOfLastEncWord=0;
  char encoding;
  bool valid, onlySpacesSinceLastWord=false;
  const int maxLen=400;
  int i;

  if(src.find("=?") < 0)
    result = src.copy();
  else {
    result.truncate(src.length());
    for (pos=src.data(), dest=result.data(); *pos; pos++)
    {
      if (pos[0]!='=' || pos[1]!='?')
      {
        *dest++ = *pos;
        if (onlySpacesSinceLastWord)
          onlySpacesSinceLastWord = (pos[0]==' ' || pos[1]=='\t');
        continue;
      }
      beg = pos+2;
      end = beg;
      valid = TRUE;
      // parse charset name
      declaredCS="";
      for (i=2,pos+=2; i<maxLen && (*pos!='?'&&(ispunct(*pos)||isalnum(*pos))); i++) {
        declaredCS+=(*pos);
        pos++;
      }
      if (*pos!='?' || i<4 || i>=maxLen) valid = FALSE;
      else
      {
        // get encoding and check delimiting question marks
        encoding = toupper(pos[1]);
        if (pos[2]!='?' || (encoding!='Q' && encoding!='B'))
          valid = FALSE;
        pos+=3;
        i+=3;
      }
      if (valid)
      {
        mid = pos;
        // search for end of encoded part
        while (i<maxLen && *pos && !(*pos=='?' && *(pos+1)=='='))
        {
          i++;
          pos++;
        }
        end = pos+2;//end now points to the first char after the encoded string
        if (i>=maxLen || !*pos) valid = FALSE;
      }

      if (valid) {
        // cut all linear-white space between two encoded words
        if (onlySpacesSinceLastWord)
          dest=endOfLastEncWord;

        if (mid < pos) {
          str = QCString(mid, (int)(pos - mid + 1));
          if (encoding == 'Q')
          {
            // decode quoted printable text
            for (i=str.length()-1; i>=0; i--)
              if (str[i]=='_') str[i]=' ';
            str = KCodecs::quotedPrintableDecode(str);
          }
          else
          {
            str = KCodecs::base64Decode(str);
          }
          for (i=0; str[i]; i++) {
            *dest++ = str[i];
          }
        }

        endOfLastEncWord=dest;
        onlySpacesSinceLastWord=true;

        pos = end -1;
      }
      else
      {
        pos = beg - 2;
        *dest++ = *pos++;
        *dest++ = *pos;
      }
    }
    *dest = '\0';
  }

  //find suitable QTextCodec
  QTextCodec *codec=0;
  bool ok=true;
  if (forceCS || declaredCS.isEmpty()) {
    codec=KGlobal::charsets()->codecForName(defaultCS);
    (*usedCS)=cachedCharset(defaultCS);
  }
  else {
    codec=KGlobal::charsets()->codecForName(declaredCS, ok);
    if(!ok) {     //no suitable codec found => use default charset
      codec=KGlobal::charsets()->codecForName(defaultCS);
      (*usedCS)=cachedCharset(defaultCS);
    }
    else
      (*usedCS)=cachedCharset(declaredCS);
  }

  return codec->toUnicode(result.data(), result.length());
}


QCString encodeRFC2047String(const QString &src, const char *charset,
			     bool addressHeader, bool allow8BitHeaders)
{
  QCString encoded8Bit, result, usedCS;
  unsigned int start=0,end=0;
  bool nonAscii=false, ok=true, useQEncoding=false;
  QTextCodec *codec=0;

  usedCS=charset;
  codec=KGlobal::charsets()->codecForName(usedCS, ok);

  if(!ok) {
    //no codec available => try local8Bit and hope the best ;-)
    usedCS=KGlobal::locale()->charset().latin1();
    codec=KGlobal::charsets()->codecForName(usedCS, ok);
  }

  if (usedCS.find("8859-")>=0)  // use "B"-Encoding for non iso-8859-x charsets
    useQEncoding=true;

  encoded8Bit=codec->fromUnicode(src);

  if(allow8BitHeaders)
    return encoded8Bit;

  for (unsigned int i=0; i<encoded8Bit.length(); i++) {
    if (encoded8Bit[i]==' ')    // encoding starts at word boundaries
      start = i+1;

    // encode escape character, for japanese encodings...
    if ((encoded8Bit[i]<0) || (encoded8Bit[i] == '\033') ||
        (addressHeader && (strchr("\"()<>@,.;:\\[]=",encoded8Bit[i])!=0))) {
      end = start;   // non us-ascii char found, now we determine where to stop encoding
      nonAscii=true;
      break;
    }
  }

  if (nonAscii) {
    while ((end<encoded8Bit.length())&&(encoded8Bit[end]!=' '))  // we encode complete words
      end++;

    for (unsigned int x=end;x<encoded8Bit.length();x++)
      if ((encoded8Bit[x]<0) || (encoded8Bit[x] == '\033') ||
          (addressHeader && (strchr("\"()<>@,.;:\\[]=",encoded8Bit[x])!=0))) {
        end = encoded8Bit.length();     // we found another non-ascii word

      while ((end<encoded8Bit.length())&&(encoded8Bit[end]!=' '))  // we encode complete words
        end++;
    }

    result = encoded8Bit.left(start)+"=?"+usedCS;

    if (useQEncoding) {
      result += "?Q?";

      char c,hexcode;                       // implementation of the "Q"-encoding described in RFC 2047
      for (unsigned int i=start;i<end;i++) {
        c = encoded8Bit[i];
        if (c == ' ')       // make the result readable with not MIME-capable readers
          result+='_';
        else
          if (((c>='a')&&(c<='z'))||      // paranoid mode, we encode *all* special characters to avoid problems
              ((c>='A')&&(c<='Z'))||      // with "From" & "To" headers
              ((c>='0')&&(c<='9')))
            result+=c;
          else {
            result += "=";                 // "stolen" from KMail ;-)
            hexcode = ((c & 0xF0) >> 4) + 48;
            if (hexcode >= 58) hexcode += 7;
            result += hexcode;
            hexcode = (c & 0x0F) + 48;
            if (hexcode >= 58) hexcode += 7;
            result += hexcode;
          }
      }
    } else {
      result += "?B?"+KCodecs::base64Encode(encoded8Bit.mid(start,end-start), false);
    }

    result +="?=";
    result += encoded8Bit.right(encoded8Bit.length()-end);
  }
  else
    result = encoded8Bit;

  return result;
}

QCString uniqueString()
{
  static char chars[] = "0123456789abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  time_t now;
  QCString ret;
  char p[11];
  int pos, ran;
  unsigned int timeval;

  p[10]='\0';
  now=time(0);
  ran=1+(int) (1000.0*rand()/(RAND_MAX+1.0));
  timeval=(now/ran)+getpid();

  for(int i=0; i<10; i++){
    pos=(int) (61.0*rand()/(RAND_MAX+1.0));
    //kdDebug(5003) << pos << endl;
    p[i]=chars[pos];
  }
  ret.sprintf("%d.%s", timeval, p);

  return ret;
}


QCString multiPartBoundary()
{
  QCString ret;
  ret="nextPart"+uniqueString();
  return ret;
}

QCString extractHeader(const QCString &src, const char *name)
{
  QCString n=QCString(name)+": ";
  int pos1=-1, pos2=0, len=src.length()-1;
  bool folded(false);

  if (n.lower() == src.left(n.length()).lower()) {
    pos1 = 0;
  } else {
    n.prepend("\n");
    pos1 = src.find(n,0,false);
  }

  if (pos1>-1) {    //there is a header with the given name
    pos1+=n.length(); //skip the name
    pos2=pos1;

    if (src[pos2]!='\n') {  // check if the header is not empty
      while(1) {
        pos2=src.find("\n", pos2+1);
        if(pos2==-1 || pos2==len || ( src[pos2+1]!=' ' && src[pos2+1]!='\t') ) //break if we reach the end of the string, honor folded lines
          break;
        else
          folded = true;
      }
    }

    if(pos2<0) pos2=len+1; //take the rest of the string

    if (!folded)
      return src.mid(pos1, pos2-pos1);
    else
      return (src.mid(pos1, pos2-pos1).replace(QRegExp("\\s*\\n\\s*")," "));
  }
  else {
    return QCString(0); //header not found
  }
}


QCString CRLFtoLF(const QCString &s)
{
  QCString ret=s.copy();
  ret.replace(QRegExp("\\r\\n"), "\n");
  return ret;
}


QCString CRLFtoLF(const char *s)
{
  QCString ret=s;
  ret.replace(QRegExp("\\r\\n"), "\n");
  return ret;
}


QCString LFtoCRLF(const QCString &s)
{
  QCString ret=s.copy();
  ret.replace(QRegExp("\\n"), "\r\n");
  return ret;
}


void removeQuots(QCString &str)
{
  bool inQuote=false;

  for (int i=0; i < (int)str.length(); i++) {
    if (str[i] == '"') {
      str.remove(i,1);
      i--;
      inQuote = !inQuote;
    } else {
      if (inQuote && (str[i] == '\\'))
        str.remove(i,1);
    }
  }
}


void removeQuots(QString &str)
{
  bool inQuote=false;

  for (int i=0; i < (int)str.length(); i++) {
    if (str[i] == '"') {
      str.remove(i,1);
      i--;
      inQuote = !inQuote;
    } else {
      if (inQuote && (str[i] == '\\'))
        str.remove(i,1);
    }
  }
}


void addQuotes(QCString &str, bool forceQuotes)
{
  bool needsQuotes=false;
  for (unsigned int i=0; i < str.length(); i++) {
    if (strchr("()<>@,.;:[]=\\\"",str[i])!=0)
      needsQuotes = true;
    if (str[i]=='\\' || str[i]=='\"') {
      str.insert(i, '\\');
      i++;
    }
  }

  if (needsQuotes || forceQuotes) {
    str.insert(0,'\"');
    str.append("\"");
  }
}




} // namespace KMime
