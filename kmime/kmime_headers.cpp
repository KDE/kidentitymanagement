/*
    knheaders.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
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
#include <qstringlist.h>

#include <kglobal.h>
#include <krfcdate.h>

#include "kmime_util.h"
#include "kmime_content.h"
#include "kmime_headers.h"
#include "kqcstringsplitter.h"

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

//-----</Base>---------------------------------



//-----<Generic>-------------------------------

Generic::Generic(const char *t)
  : Base(), t_ype(0)
{
  setType(t);
}


Generic::Generic(const char *t, Content *p)
  : Base(p), t_ype(0)
{
  setType(t);
}


Generic::Generic(const char *t, Content *p, const QCString &s)
  : Base(p), t_ype(0)
{
  setType(t);
  from7BitString(s);
}


Generic::Generic(const char *t, Content *p, const QString &s, const QCString &cs)
  : Base(p), t_ype(0)
{
  setType(t);
  fromUnicodeString(s, cs);
}


Generic::~Generic()
{
  delete[] t_ype;
}


void Generic::from7BitString(const QCString &s)
{
  u_nicode=decodeRFC2047String(s, &e_ncCS, defaultCS(), forceCS());
}


QCString Generic::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+encodeRFC2047String(u_nicode, e_ncCS) );
  else
    return encodeRFC2047String(u_nicode, e_ncCS);
}


void Generic::fromUnicodeString(const QString &s, const QCString &cs)
{
  u_nicode=s;
  e_ncCS=cachedCharset(cs);
}


QString Generic::asUnicodeString()
{
  return u_nicode;
}


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



//-----<Subject>-------------------------------

void Subject::from7BitString(const QCString &s)
{
  s_ubject=decodeRFC2047String(s, &e_ncCS, defaultCS(), forceCS());
}


QCString Subject::as7BitString(bool incType)
{
  if(incType)
    return (typeIntro()+encodeRFC2047String(s_ubject, e_ncCS));
  else
    return encodeRFC2047String(s_ubject, e_ncCS);
}


void Subject::fromUnicodeString(const QString &s, const QCString &cs)
{
  s_ubject=s;
  e_ncCS=cachedCharset(cs);
}


QString Subject::asUnicodeString()
{
  return s_ubject;
}

//-----</Subject>------------------------------



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


//-----<Organization>--------------------------

void Organization::from7BitString(const QCString &s)
{
  o_rga=decodeRFC2047String(s, &e_ncCS, defaultCS(), forceCS());
}


QCString Organization::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+encodeRFC2047String(o_rga, e_ncCS) );
  else
    return encodeRFC2047String(o_rga, e_ncCS);
}


void Organization::fromUnicodeString(const QString &s, const QCString &cs)
{
  o_rga=s;
  e_ncCS=cachedCharset(cs);
}


QString Organization::asUnicodeString()
{
  return o_rga;
}

//-----</Organization>-------------------------



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



//-----<CDescription>--------------------------

void CDescription::from7BitString(const QCString &s)
{
  d_esc=decodeRFC2047String(s, &e_ncCS, defaultCS(), forceCS());
}


QCString CDescription::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+encodeRFC2047String(d_esc, e_ncCS) );
  else
    return encodeRFC2047String(d_esc, e_ncCS);
}


void CDescription::fromUnicodeString(const QString &s, const QCString &cs)
{
  d_esc=s;
  e_ncCS=cachedCharset(cs);
}


QString CDescription::asUnicodeString()
{
  return d_esc;
}

//-----</CDescription>-------------------------

}; // namespace Headers
}; // namespace KMime
