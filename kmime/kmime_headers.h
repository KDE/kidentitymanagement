/*
    kmime_headers.h

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
#ifndef __KMIME_HEADERS_H__
#define __KMIME_HEADERS_H__

#include <time.h>

#include <qstring.h>
#include <qstrlist.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qasciidict.h>
#if QT_VERSION >= 290
#  include <qptrlist.h>
#else
// remove once Qt3 becomes mandatory
#  include <qlist.h>
#  define QPtrList QList
#  define QPtrListIterator QListIterator
#endif

namespace KMime {

//forward declaration
class Content;


namespace Headers {

enum contentCategory    { CCsingle,
                          CCcontainer,
                          CCmixedPart,
                          CCalternativePart };

enum contentEncoding    { CE7Bit,
                          CE8Bit,
                          CEquPr,
                          CEbase64,
                          CEuuenc,
                          CEbinary };

enum contentDisposition { CDinline,
                          CDattachment,
                          CDparallel };

//often used charset
static const QCString Latin1("ISO-8859-1");




/** Baseclass of all header-classes. It represents a
    header-field as described in RFC-822.  */
class Base {

  public:
    typedef QPtrList<Base> List;

    /** Create an empty header. */
    Base() : p_arent(0), e_ncCS(0) {}

    /** Create an empty header with a parent-content. */
    Base(KMime::Content *parent) : p_arent(parent), e_ncCS(0) {}

    /** Destructor */
    virtual ~Base()  {}

    /** Return the parent of this header. */
    KMime::Content* parent()  { return p_arent; }

    /** Set the parent for this header. */
    void setParent(KMime::Content *p)  { p_arent=p; }

    /** Parse the given string. Take care of RFC2047-encoded
	strings. A default charset is given. If the last parameter
	is true the default charset is used in any case */
    virtual void from7BitString(const QCString&)  {}

    /** Return the encoded header. The parameter specifies
	whether the header-type should be included. */
    virtual QCString as7BitString(bool=true)  { return QCString(); }

    /** Return the charset that is used for RFC2047-encoding */
    QCString rfc2047Charset();

    /** Set the charset for RFC2047-encoding */
    void setRFC2047Charset(const QCString &cs);

    /** Return the default charset */
    QCString defaultCS();

    /** Return if the default charset is mandatory */
    bool forceCS();

    /** Parse the given string and set the charset. */
    virtual void fromUnicodeString(const QString&, const QCString&)  {}

    /** Return the decoded content of the header without
       the header-type. */
    virtual QString asUnicodeString()  { return QString(); }

    /** Delete */
    virtual void clear()  {}

    /** Do we have data? */
    virtual bool isEmpty()  { return false; }

    /** Return the type of this header (e.g. "From") */
    virtual const char* type()  { return ""; }

    /** Check if this header is of type t. */
    bool is(const char* t)  { return (strcasecmp(t, type())==0); }

    /** Check if this header is a MIME header */
    bool isMimeHeader()  { return (strncasecmp(type(), "Content-", 8)==0); }

    /** Check if this header is a X-Header */
    bool isXHeader()  { return (strncmp(type(), "X-", 2)==0); }

  protected:
    QString parseEncodedWord( const QCString & str, int & pos, bool & ok );
    
    QCString typeIntro()  { return (QCString(type())+": "); }

    const char *e_ncCS;
    Content *p_arent;

};

/** Abstract base class for unstructured header fields
    (e.g. "Subject", "Comment", "Content-description").
    
    Features: Decodes the header according to RFC2047, incl. RFC2231
    extensions to encoded-words.

    Subclasses need only re-implement @p const @p char* @p type().

    A macro to automate this is named
    <pre>
    MK_TRIVIAL_GUnstructured_SUBCLASS(classname,headername);
    </pre>

    The @ref ContentDescription class then reads:
    <pre>
    MK_TRIVIAL_GUnstructured_SUBCLASS(ContentDescription,Content-Description);
    </pre>
*/

class GUnstructured : public Base {

public:
  GUnstructured() : Base()  {}
  GUnstructured( Content * p ) : Base( p ) {}
  GUnstructured( Content * p, const QCString & s )
    : Base( p ) { from7BitString(s); }
  GUnstructured( Content * p, const QString & s, const QCString & cs )
    : Base( p )  { fromUnicodeString( s, cs ); }
  ~GUnstructured()  {}

  virtual void from7BitString( const QCString& str );
  virtual QCString as7BitString( bool withHeaderType=true );

  virtual void fromUnicodeString( const QString & str,
				  const QCString & suggestedCharset);
  virtual QString asUnicodeString();

  virtual void clear()            { d_ecoded.truncate(0); }
  virtual bool isEmpty()          { return (d_ecoded.isEmpty()); }

private:
  QString d_ecoded;
};

class GStructured : public Base {
public:
  GStructured() : Base()  {}
  GStructured( Content * p ) : Base( p ) {}
  GStructured( Content * p, const QCString & s )
    : Base( p ) { from7BitString(s); }
  GStructured( Content * p, const QString & s, const QCString & cs )
    : Base( p )  { fromUnicodeString( s, cs ); }
  ~GStructured()  {}

  
protected:
  /** Types of tokens to be found in structured header fields:
      <pre>
          Type     |def'ed in| ... as
      -------------+------------------------------------------
      Token        | rfc2045 | 1*<CHAR w/o SPACE, CTLs and tspecials>
      TSpecial     | rfc2045 | Special / "/" / "?" / "="
      Atom         | rfc822  | 1*atext
                   |         | atext := <CHAR w/o specials, SPACE and CTLs>
		   | KMime   | 1*(token / "/" / "?" / "=")
      Special      | rfc822  | "()<>@,;:.[]" / <"> / "\"
      Comment      | rfc822  | "(" *(ctext / quoted-pair / comment) ")"
                   |         | ctext := <CHAR w/o "()\", CR, but LWSP>
      QuotedString | rfc822  | <"> *(qtext/quoted-pair) <">
                   |         | qtext := <CHAR w/o <">, "\", CR, but LWSP>
      DomainLiteral| rfc822  | "[" *(dtext / quoted-pair) "]"
                   |         | dtext := <CHAR w/o "[]\", CR, but LWSP>
      EncodedWord  | rfc2047 | "=?" charset "?" ( "q" / "b" ) "?" encoded-text "?="
      Phrase       | rfc822  | 1*word
                   | rfc2047 | 1*(word / encoded-word)
      DotAtom      | rfc2822 | 1*atext *("." 1*atext)
                   | KMime   | atom *("." atom) ["."]
      Word         | rfc822  | atom / quoted-string
                   | KMime   | atom / token / quoted-string
      </pre>
   */
  enum TokenType { None = 0x00, Atom = 0x001, Special = 0x002, Comment = 0x004,
		   QuotedString = 0x008, DomainLiteral = 0x010,
		   EncodedWord = 0x020, Phrase = 0x040,
		   Token = 0x080, TSpecial = 0x100, DotAtom = 0x200,
		   Word = Token|Atom|QuotedString,
		   All = Word|Special|Comment|DomainLiteral|EncodedWord|Phrase|TSpecial|DotAtom};
  /** Starting at index @p pos in @p source, return the next token of
      a type specified by @p tt (can be or'ed).

      Different tokens behave differently when they are disallowed. In
      particular, Comment is considered part of CFWS and simply
      ignored (but still parsed correctly!). If you choose to disallow
      QuotedString or DomainLiteral, you will get them as a list of
      Atom's and Specials, one per call. Note that this might lead to
      parse errors when you change the type mask later on to allow
      them again. Tokens can't be disallowed. If you forbid TSpecials,
      you must allow Atoms and Specials. The lowest syntactic entities
      are then Atom and Special (ie. like in rfc822). If you forbid
      EncodedWords, you'll get them as an Atom, or, if you allowed
      TSpecials, as a list of TSpecials and Tokens. If you forbit
      Phrase, getToken doesn't condense consecutive Word's into a
      Phrase, but returns them one by one.  If you disallow DotAtom,
      you'll get them as a list of Atoms and Specials.

      NOTE: Due to keeping the tokenizer lookahead (and complexity)
      under control, you must take into account several issues with
      some of the token types listed above. In particular, the
      definition of a dot-atom deviates from the RFC one in that it is
      allowed to have a trailing dot. The lookahead necessary to make
      sure that this case does not occur would be unlimited (remember
      that rfc822 allows arbitrary CFWS in between dot-atoms, though
      rfc2822 forbids that). You should therefore exclude dot-atoms
      from the list of requested types unless you really expect one,
      and check for trailing dots if you get one.

      Also, a sequence of encoded-words is condensed into a single
      one. This is because there are special rules for when encoded
      words are adjacent.

      @param source source string
      @param pos    in: starting position; out: new position
      @param tt     in: types of tokens requested; out: type of token returned
      @param CRLF   if true, the mail is in canonical CRLF form, so the parsers
                    can handle lonely '\n' and '\r' according to rfc822.
                    If false, the mail is in Unix-native format and the parser
		    will eat every occurence of unescaped '\n' followed by
		    SPACE or HTAB as meaning line folding.
      @return the decoded token or @ref QString::null if there are no
              more tokens
      @see tokenType */
  QString getToken( const QCString & source, int & pos,
		    TokenType & tt, bool CRLF=false );

  /** Used by @ref getToken to parse a @p QuotedString, a
      @p DomainLiteral or a Comment.

      This function parses a generic quoted string, ie. one that is
      begun (@p openChar) and ended (@p closeChar) by a single char
      each and may contain FWS and quoted-pairs.

      If it returns, you should check for the following conditions:
      @li @p src[pos-1] == @p closeChar
          OK, the element was parsed to it's end. Depending on whether
	  you want nesting or not, you can restart the function at
	  @p pos or stop.
      @li @p src[pos-1] == @p openChar
          The parser encountered another opening char. Depending on
	  whether you want nesting or not, you can restart the function
	  at @p pos or create an error.
      @li @p src[pos-1] == 0
          ### Premature end of the generic quoted string. You should
	  perhaps scan the whole thing again as an atom.

      @param source    source string
      @param pos       in: starting position (assumed to be after
                       the opening <">); out: new position (after
		       the closing <">).
      @param openChar  The character that begins a new generic
                       QuotedString (<"> for quoted-string, "[" for
		       domain-literal and "(" for comment).
      @param closeChar The character that closes a generic
                       QuotedString (<"> for quoted-string, "]" for
		       domain-literal and ")" for comment).
      @param isCRLF    see the parameter to @ref getToken of the same name 
      @return the decoded part of @p src.
      @see getToken
      @internal
  */
  QString parseGenericQuotedString( const QCString & src, int & pos,
		const char openChar, const char closeChar, bool CRLF=false );

};

/** Represents an arbitrary header, that can contain
    any header-field.
    Adds a type over @ref GUnstructured.
    @see GUnstructured
*/
class Generic : public GUnstructured {

  public:
    Generic() : GUnstructured(), t_ype(0) {}
    Generic(const char *t) : GUnstructured(), t_ype(0) { setType(t); }
    Generic(const char *t, Content *p) : GUnstructured( p ), t_ype(0) { setType(t); }
    Generic(const char *t, Content *p, const QCString &s)
      : GUnstructured( p, s ), t_ype(0) { setType(t); }
    Generic(const char *t, Content *p, const QString &s, const QCString &cs)
      : GUnstructured( p, s, cs ), t_ype(0) { setType(t); }
    ~Generic() { delete[] t_ype; }

    virtual void clear()            { delete[] t_ype; GUnstructured::clear(); }
    virtual bool isEmpty()          { return (t_ype==0 || GUnstructured::isEmpty()); }
    virtual const char* type()      { return t_ype; }
    void setType(const char *type);

  protected:
    char *t_ype;

};


/** Represents a "Message-Id" header */
class MessageID : public Base {

  public:
    MessageID() : Base()  {}
    MessageID(Content *p) : Base(p) {}
    MessageID(Content *p, const QCString &s) : Base(p) { from7BitString(s); }
    MessageID(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~MessageID()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { m_id.resize(0); }
    virtual bool isEmpty()          { return (m_id.isEmpty()); }
    virtual const char* type()      { return "Message-Id"; }

    void generate(const QCString &fqdn);

  protected:
    QCString m_id;

};


/** Represents a "Control" header */
class Control : public Base {

  public:
    Control() : Base()  {}
    Control(Content *p) : Base(p)  {}
    Control(Content *p, const QCString &s) : Base(p) { from7BitString(s); }
    Control(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~Control()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { c_trlMsg.truncate(0); }
    virtual bool isEmpty()          { return (c_trlMsg.isEmpty()); }
    virtual const char* type()      { return "Control"; }

    bool isCancel()                 { return (c_trlMsg.find("cancel", 0, false)!=-1); }

  protected:
    QCString c_trlMsg;

};


/** Represents a "Supersedes" header */
class Supersedes : public MessageID {

  public:
    Supersedes() : MessageID()  {}
    Supersedes(Content *p) : MessageID(p)  {}
    Supersedes(Content *p, const QCString &s) : MessageID(p,s)  {}
    Supersedes(Content *p, const QString &s)  : MessageID(p,s)  {}
    ~Supersedes()                   {}

    virtual const char* type()      { return "Supersedes"; }

};


/** Represents a "Subject" header */
class Subject : public GUnstructured {

  public:
    Subject() : GUnstructured()  {}
    Subject( Content * p ) : GUnstructured( p )  {}
    Subject( Content * p, const QCString & s ) : GUnstructured( p, s ) {}
    Subject( Content * p, const QString & s, const QCString & cs )
      : GUnstructured( p, s, cs ) {}
    ~Subject()  {}

    virtual const char* type() { return "Subject"; }

    bool isReply() {
      return ( asUnicodeString().find( QString("Re:"), 0, false ) == 0 );
    }
};


/** This class encapsulates an address-field, containing
    an email-adress and a real name */
class AddressField : public Base {

  public:
    AddressField() : Base()  {}
    AddressField(Content *p) : Base(p)  {}
    AddressField(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    AddressField(Content *p, const QString &s, const QCString &cs) : Base(p)  { fromUnicodeString(s, cs); }
    AddressField(const AddressField &a):  Base(a.p_arent)  { n_ame=a.n_ame; e_mail=a.e_mail.copy(); e_ncCS=a.e_ncCS; }
    ~AddressField()  {}

    AddressField& operator=(const AddressField &a)  { n_ame=a.n_ame; e_mail=a.e_mail.copy(); e_ncCS=a.e_ncCS; return (*this); }

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString &cs);
    virtual QString asUnicodeString();
    virtual void clear()              { n_ame.truncate(0); e_mail.resize(0); }
    virtual bool isEmpty()            { return (e_mail.isEmpty() && n_ame.isEmpty()); }

    bool hasName()                    { return ( !n_ame.isEmpty() ); }
    bool hasEmail()                   { return ( !e_mail.isEmpty() ); }
    QString name()                    { return n_ame; }
    QCString nameAs7Bit();
    QCString email()                  { return e_mail; }
    void setName(const QString &s)    { n_ame=s; }
    void setNameFrom7Bit(const QCString &s);
    void setEmail(const QCString &s)  { e_mail=s; }

  protected:
    QString n_ame;
    QCString e_mail;
};
typedef QPtrList<AddressField> AddressList;


/** Represent a "From" header */
class From : public AddressField {

  public:
    From() : AddressField()  {}
    From(Content *p) : AddressField(p)  {}
    From(Content *p, const QCString &s) : AddressField(p,s)  {}
    From(Content *p, const QString &s, const QCString &cs) : AddressField(p,s,cs)  {}
    ~From()  {}

    virtual const char* type()      { return "From"; }
};


/** Represents a "Reply-To" header */
class ReplyTo : public AddressField {

  public:
    ReplyTo() : AddressField()  {}
    ReplyTo(Content *p) : AddressField(p)  {}
    ReplyTo(Content *p, const QCString &s) : AddressField(p,s)  {}
    ReplyTo(Content *p, const QString &s, const QCString &cs) : AddressField(p,s,cs)  {}
    ~ReplyTo()  {}

    virtual const char* type()      { return "Reply-To"; }

};


/** Represents a "Mail-Copies-To" header
    http://www.newsreaders.com/misc/mail-copies-to.html */
class MailCopiesTo : public AddressField {

  public:
    MailCopiesTo() : AddressField()  {}
    MailCopiesTo(Content *p) : AddressField(p)  {}
    MailCopiesTo(Content *p, const QCString &s) : AddressField(p,s)  {}
    MailCopiesTo(Content *p, const QString &s, const QCString &cs) : AddressField(p,s,cs)  {}
    ~MailCopiesTo()  {}

    bool isValid();
    bool alwaysCopy();
    bool neverCopy();

    virtual const char* type()      { return "Mail-Copies-To"; }

};


/** Represents a "Organization" header */
class Organization : public GUnstructured {

  public:
    Organization() : GUnstructured() {}
    Organization( Content * p ) : GUnstructured( p ) {}
    Organization( Content * p, const QCString & s )
      : GUnstructured( p, s ) {};
    Organization( Content * p, const QString & s, const QCString & cs)
      : GUnstructured( p, s, cs ) {}
    ~Organization()  {}

    virtual const char* type()      { return "Organization"; }

};


/** Represents a "Date" header */
class Date : public Base {

  public:
    Date() : Base(), t_ime(0)  {}
    Date(Content *p) : Base(p), t_ime(0)  {}
    Date(Content *p, time_t t) : Base(p), t_ime(t)  {}
    Date(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    Date(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~Date()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { t_ime=0; }
    virtual bool isEmpty()          { return (t_ime==0); }
    virtual const char* type()      { return "Date"; }

    time_t unixTime()               { return t_ime; }
    void setUnixTime(time_t t)      { t_ime=t; }
    void setUnixTime()              { t_ime=time(0); }
    QDateTime qdt();
    int ageInDays();
    
  protected:
    time_t t_ime;

};


/** Represents a "To" header */
class To : public Base {

  public:
    To() : Base(),a_ddrList(0)  {}
    To(Content *p) : Base(p),a_ddrList(0)  {}
    To(Content *p, const QCString &s) : Base(p),a_ddrList(0)  { from7BitString(s); }
    To(Content *p, const QString &s, const QCString &cs) : Base(p),a_ddrList(0)  { fromUnicodeString(s,cs); }
    ~To()  { delete a_ddrList; }

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString &cs);
    virtual QString asUnicodeString();
    virtual void clear()            { delete a_ddrList; a_ddrList=0; }
    virtual bool isEmpty()          { return (!a_ddrList || a_ddrList->isEmpty()
                                              || a_ddrList->first()->isEmpty()); }
    virtual const char* type()      { return "To"; }

    void addAddress(const AddressField &a);
    void emails(QStrList *l);

  protected:
    AddressList *a_ddrList;

};


/** Represents a "CC" header */
class CC : public To {

  public:
    CC() : To()  {}
    CC(Content *p) : To(p)  {}
    CC(Content *p, const QCString &s) : To(p,s)  {}
    CC(Content *p, const QString &s, const QCString &cs) : To(p,s,cs)  {}
    ~CC()  {}

    virtual const char* type()      { return "CC"; }

};


/** Represents a "BCC" header */
class BCC : public To {

  public:
    BCC() : To()  {}
    BCC(Content *p) : To(p)  {}
    BCC(Content *p, const QCString &s) : To(p,s)  {}
    BCC(Content *p, const QString &s, const QCString &cs) : To(p,s,cs)  {}
    ~BCC()  {}

    virtual const char* type()      { return "BCC"; }

};


/** Represents a "Newsgroups" header */
class Newsgroups : public Base {

  public:
    Newsgroups() : Base()  {}
    Newsgroups(Content *p) : Base(p)  {}
    Newsgroups(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    Newsgroups(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~Newsgroups()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { g_roups.resize(0); }
    virtual bool isEmpty()          { return g_roups.isEmpty(); }
    virtual const char* type()      { return "Newsgroups"; }

    QCString firstGroup();
    bool isCrossposted()            { return ( g_roups.find(',')>-1 ); }
    QStringList getGroups();

  protected:
    QCString g_roups;

};


/** Represents a "Followup-To" header */
class FollowUpTo : public Newsgroups {

  public:
    FollowUpTo() : Newsgroups()  {}
    FollowUpTo(Content *p) : Newsgroups(p)  {}
    FollowUpTo(Content *p, const QCString &s) : Newsgroups(p,s)  {}
    FollowUpTo(Content *p, const QString &s) : Newsgroups(p,s)  {}
    ~FollowUpTo()  {}

    virtual const char* type()        { return "Followup-To"; }

};


/** Represents a "Lines" header */
class Lines : public Base {

  public:
    Lines() : Base(),l_ines(-1)  {}
    Lines(Content *p) : Base(p),l_ines(-1)  {}
    Lines(Content *p, unsigned int i) : Base(p),l_ines(i)  {}
    Lines(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    Lines(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~Lines()                 {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { l_ines=-1; }
    virtual bool isEmpty()          { return (l_ines==-1); }
    virtual const char* type()      { return "Lines"; }

    int numberOfLines()             { return l_ines; }
    void setNumberOfLines(int i)    { l_ines=i; }

  protected:
    int l_ines;

};


/** Represents a "References" header */
class References : public Base {

  public:
    References() : Base(),p_os(-1)  {}
    References(Content *p) : Base(p),p_os(-1)  {}
    References(Content *p, const QCString &s) : Base(p),p_os(-1)  { from7BitString(s); }
    References(Content *p, const QString &s) : Base(p),p_os(-1)  { fromUnicodeString(s, Latin1); }
    ~References()                 {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { r_ef.resize(0); p_os=0; }
    virtual bool isEmpty()          { return (r_ef.isEmpty()); }
    virtual const char* type()      { return "References"; }

    int count();
    QCString first();
    QCString next();
    QCString at(unsigned int i);
    void append(const QCString &s);

  protected:
    QCString r_ef;
    int p_os;

};


/** Represents a "User-Agent" header */
class UserAgent : public Base {

  public:
    UserAgent() : Base()  {}
    UserAgent(Content *p) : Base(p)  {}
    UserAgent(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    UserAgent(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~UserAgent()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { u_agent.resize(0); }
    virtual bool isEmpty()          { return (u_agent.isEmpty()); }
    virtual const char* type()      { return "User-Agent"; }

  protected:
    QCString u_agent;

};


/** Represents a "Content-Type" header */
class ContentType : public Base {

  public:
    ContentType() : Base(),m_imeType("invalid/invalid"),c_ategory(CCsingle)  {}
    ContentType(Content *p) : Base(p),m_imeType("invalid/invalid"),c_ategory(CCsingle)  {}
    ContentType(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    ContentType(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~ContentType()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { m_imeType.resize(0); p_arams.resize(0); }
    virtual bool isEmpty()          { return (m_imeType.isEmpty()); }
    virtual const char* type()      { return "Content-Type"; }


    //mime-type handling
    QCString mimeType()                     { return m_imeType; }
    QCString mediaType();
    QCString subType();
    void setMimeType(const QCString &s);
    bool isMediatype(const char *s);
    bool isSubtype(const char *s);
    bool isText();
    bool isPlainText();
    bool isHTMLText();
    bool isImage();
    bool isMultipart();
    bool isPartial();

    //parameter handling
    QCString charset();
    void setCharset(const QCString &s);
    QCString boundary();
    void setBoundary(const QCString &s);
    QString name();
    void setName(const QString &s, const QCString &cs);
    QCString id();
    void setId(const QCString &s);
    int partialNumber();
    int partialCount();
    void setPartialParams(int total, int number);

    //category
    contentCategory category()            { return c_ategory; }
    void setCategory(contentCategory c)   { c_ategory=c; }

  protected:
    QCString getParameter(const char *name);
    void setParameter(const QCString &name, const QCString &value, bool doubleQuotes=false);
    QCString m_imeType, p_arams;
    contentCategory c_ategory;

};


/** Represents a "Content-Transfer-Encoding" header */
class CTEncoding : public Base {

  public:
    CTEncoding() : Base(),c_te(CE7Bit),d_ecoded(true)  {}
    CTEncoding(Content *p) : Base(p),c_te(CE7Bit),d_ecoded(true)  {}
    CTEncoding(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    CTEncoding(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~CTEncoding()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { d_ecoded=true; c_te=CE7Bit; }
    virtual const char* type()      { return "Content-Transfer-Encoding"; }

    contentEncoding cte()                   { return c_te; }
    void setCte(contentEncoding e)          { c_te=e; }
    bool decoded()                          { return d_ecoded; }
    void setDecoded(bool d=true)            { d_ecoded=d; }
    bool needToEncode()                     { return (d_ecoded && (c_te==CEquPr || c_te==CEbase64)); }

  protected:
    contentEncoding c_te;
    bool d_ecoded;

};


/** Represents a "Content-Disposition" header */
class CDisposition : public Base {

  public:
    CDisposition() : Base(),d_isp(CDinline)  {}
    CDisposition(Content *p) : Base(p),d_isp(CDinline)  {}
    CDisposition(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    CDisposition(Content *p, const QString &s, const QCString &cs) : Base(p)  { fromUnicodeString(s, cs); }
    ~CDisposition()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString &cs);
    virtual QString asUnicodeString();
    virtual void clear()            { f_ilename.truncate(0); d_isp=CDinline; }
    virtual const char* type()      { return "Content-Disposition"; }

    contentDisposition disposition()          { return d_isp; }
    void setDisposition(contentDisposition d) { d_isp=d; }
    bool isAttachment()                       { return (d_isp==CDattachment); }

    QString filename()                        { return f_ilename; }
    void setFilename(const QString &s)        { f_ilename=s; }

  protected:
    contentDisposition d_isp;
    QString f_ilename;

};


/** Represents a "Content-Description" header */
class CDescription : public GUnstructured {

  public:
    CDescription() : GUnstructured()  {}
    CDescription( Content * p ) : GUnstructured( p )  {}
    CDescription( Content * p, const QCString & s )
      : GUnstructured( p, s ) {};
    CDescription( Content * p, const QString & s, const QCString & cs )
      : GUnstructured( p, s, cs ) {}
    ~CDescription()  {}
    
    virtual const char* type()      { return "Content-Description"; }
};

};  //namespace Headers

#if 0
typedef Headers::Base* (*headerCreator)(void);

/** This is a factory for KMime::Headers. You can create new header
    objects by type with @ref create and @ref upgrade an existing
    @ref Headers::Generic to a specialized header object.

    If you are a header class author, you can register your class
    (let's call it Foo) so:
    <pre>
    
    </pre>    

    @short Factory for KMime::Headers
    @author Marc Mutz <mutz@kde.org>
    @see KMime::Headers::Base KMime::Headers::Generic
*/

class HeaderFactory : public QAsciiDict<headerCreator>
{
private:
  HeaderFactory();
  ~HeaderFactory() {}
  static QAsciiDict

public:
  /** Create a new header object of type @p aType, or a fitting
      generic substitute, if available and known
  */
  static Headers::Base* create( const char* aType )
  {
    if (!s_elf)
      s_elf = new HeaderFactory;
    headerCreator * hc = (*s_elf)[aType];
    if ( !hc )
      return 0;
    else
      return (*hc)();
  }

  /** This is a wrapper around the above function, provided for
      convenience. It differs from the above only in what arguments it
      takes.
  */
  static Headers::Base* create( const QCString& aType )
  {
    return create( aType.data() );
  }

  /** Consume @p aType and build a header object that corresponds to
      the type that @p aType->type() returns.
      @param  aType    generic header to upgrade. This will be deleted
                       if necessary, so don't use references to it after
                       calling this function.
      @return A corresponding header object (if available), or a generic
      object for this kind of header (if known), or @p aType (else).
      @see Headers::Generic create
  */
  static Headers::Base* upgrade( Headers::Generic* aType ) { (void)aType; return new Headers::Base; }

};

#endif

};  //namespace KMime


#endif // __KMIME_HEADERS_H__

