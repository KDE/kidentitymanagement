#ifndef __KMIME_PARSERS__
#define __KMIME_PARSERS__

#include <qvaluelist.h>
#include <qcstring.h>
#include <qstrlist.h>

namespace KMime {

namespace Parser {

/** Helper-class: splits a multipart-message into single
    parts as described in RFC 2046
    @internal
*/
class MultiPart {
  
public:
  MultiPart(const QCString &src, const QCString &boundary);
  ~MultiPart() {};
  
  bool parse();
  QValueList<QCString> parts()    { return p_arts; }
  QCString preamble()     { return p_reamble; }
  QCString epilouge()     { return e_pilouge; }
  
protected:
  QCString s_rc, b_oundary, p_reamble, e_pilouge;
  QValueList<QCString> p_arts;
};



/** Helper-class: tries to extract the data from a possibly
    uuencoded message
    @internal
*/
class UUEncoded {

public:
  UUEncoded(const QCString &src, const QCString &subject);
  ~UUEncoded() {};

  bool parse();
  bool isPartial()            { return (p_artNr>-1 && t_otalNr>-1); }
  int partialNumber()         { return p_artNr; }
  int partialCount()          { return t_otalNr; }
  bool hasTextPart()          { return (t_ext.length()>1); }
  QCString textPart()         { return t_ext; }
  QStrList binaryParts()       { return b_ins; }
  QStrList filenames()         { return f_ilenames; }
  QStrList mimeTypes()         { return m_imeTypes; }

protected:
  QCString s_rc, t_ext, s_ubject;
  QStrList b_ins, f_ilenames, m_imeTypes;
  int p_artNr, t_otalNr;
};



}; // namespace Parser

}; // namespace KMime

#endif // __KMIME_PARSERS__
