#include "kmime_parsers.h"

#include <qregexp.h>

using namespace KMime::Parser;

namespace KMime {
namespace Parser {

MultiPart::MultiPart(const QCString &src, const QCString &boundary)
{
  s_rc=src;
  b_oundary=boundary;
}


bool MultiPart::parse()
{
  QCString b="--"+b_oundary, part;
  int pos1=0, pos2=0, blen=b.length();

  p_arts.clear();

  //find the first valid boundary
  while(1) {
    if( (pos1=s_rc.find(b, pos1))==-1 || pos1==0 || s_rc[pos1-1]=='\n' ) //valid boundary found or no boundary at all
      break;
    pos1+=blen; //boundary found but not valid => skip it;
  }

  if(pos1>-1) {
    pos1+=blen;
    if(s_rc[pos1]=='-' && s_rc[pos1+1]=='-') // the only valid boundary is the end-boundary - this message is *really* broken
      pos1=-1; //we give up
    else if( (pos1-blen)>1 ) //preamble present
      p_reamble=s_rc.left(pos1-blen);
  }


  while(pos1>-1 && pos2>-1) {

    //skip the rest of the line for the first boundary - the message-part starts here
    if( (pos1=s_rc.find('\n', pos1))>-1 ) { //now search the next linebreak
      //now find the next valid boundary
      pos2=++pos1; //pos1 and pos2 point now to the beginning of the next line after the boundary
      while(1) {
        if( (pos2=s_rc.find(b, pos2))==-1 || s_rc[pos2-1]=='\n' ) //valid boundary or no more boundaries found
          break;
        pos2+=blen; //boundary is invalid => skip it;
      }

      if(pos2==-1) { // no more boundaries found
        part=s_rc.mid(pos1, s_rc.length()-pos1); //take the rest of the string
        p_arts.append(part);
        pos1=-1;
        pos2=-1; //break;
      }
      else {
        part=s_rc.mid(pos1, pos2-pos1);
        p_arts.append(part);
        pos2+=blen; //pos2 points now to the first charakter after the boundary
        if(s_rc[pos2]=='-' && s_rc[pos2+1]=='-') { //end-boundary
          pos1=pos2+2; //pos1 points now to the character directly after the end-boundary
          if( (pos1=s_rc.find('\n', pos1))>-1 ) //skipt the rest of this line
            e_pilouge=s_rc.mid(++pos1, s_rc.length()-pos1); //everything after the end-boundary is considered as the epilouge
          pos1=-1;
          pos2=-1; //break
        }
        else {
          pos1=pos2; //the search continues ...
        }
      }
    }
  }

  return (!p_arts.isEmpty());
}


//============================================================================================


UUEncoded::UUEncoded(const QCString &src, const QCString &subject) :
  s_rc(src), s_ubject(subject), p_artNr(-1), t_otalNr(-1)
{}


bool UUEncoded::parse()
{
  int currentPos=0;
  bool success=true, firstIteration=true;

  while (success) {
    int beginPos=currentPos, uuStart=currentPos, endPos=0, lineCount=0, MCount=0, pos=0, len=0;
    bool containsBegin=false, containsEnd=false;
    QCString tmp,fileName,mimeType;

    if( (beginPos=s_rc.find(QRegExp("begin [0-9][0-9][0-9]"),currentPos))>-1 && (beginPos==0 || s_rc.at(beginPos-1)=='\n') ) {
      containsBegin=true;
      uuStart=s_rc.find('\n', beginPos);
      if(uuStart==-1) {//no more line breaks found, we give up
        success = false;
        break;
      } else
        uuStart++; //points now at the beginning of the next line
    }
      else beginPos=currentPos;

    if ( (endPos=s_rc.find("\nend",(uuStart>0)? uuStart-1:0))==-1 )
      endPos=s_rc.length(); //no end found
    else
      containsEnd=true;

    if ((containsBegin && containsEnd) || firstIteration) {

      //printf("beginPos=%d , uuStart=%d , endPos=%d\n", beginPos, uuStart, endPos);
      //all lines in a uuencoded text start with 'M'
      for(int idx=uuStart; idx<endPos; idx++)
        if(s_rc[idx]=='\n') {
          lineCount++;
          if(idx+1<endPos && s_rc[idx+1]=='M') {
            idx++;
            MCount++;
          }
        }

      //printf("lineCount=%d , MCount=%d\n", lineCount, MCount);
      if( MCount==0 || (lineCount-MCount)>10 ||
          ((!containsBegin || !containsEnd) && (MCount<15)) ) {  // harder check for splitted-articles
        success = false;
        break; //too many "non-M-Lines" found, we give up
      }

      if( (!containsBegin || !containsEnd) && s_ubject) {  // message may be split up => parse subject
	QRegExp rx("[0-9]+/[0-9]+");
#if QT_VERSION >= 290
	pos=rx.search(QString(s_ubject), 0);
	len=rx.matchedLength();
#else
        pos=rx.match(QString(s_ubject), 0, &len);
#endif // Qt2 workaround.
        if(pos!=-1) {
          tmp=s_ubject.mid(pos, len);
          pos=tmp.find('/');
          p_artNr=tmp.left(pos).toInt();
          t_otalNr=tmp.right(tmp.length()-pos-1).toInt();
        } else {
          success = false;
          break; //no "part-numbers" found in the subject, we give up
        }
      }

      //everything before "begin" is text
      if(beginPos>0)
        t_ext.append(s_rc.mid(currentPos,beginPos-currentPos));

      if(containsBegin)
        fileName = s_rc.mid(beginPos+10, uuStart-beginPos-11); //everything between "begin ### " and the next LF is considered as the filename
      else
        fileName = "";
      f_ilenames.append(fileName);
      b_ins.append(s_rc.mid(uuStart, endPos-uuStart+1)); //everything beetween "begin" and "end" is uuencoded

      //try to guess the mimetype from the file-extension
      if(!fileName.isEmpty()) {
        pos=fileName.findRev('.');
        if(pos++ != -1) {
          tmp=fileName.mid(pos, fileName.length()-pos).upper();
          if(tmp=="JPG" || tmp=="JPEG")       mimeType="image/jpeg";
          else if(tmp=="GIF")                 mimeType="image/gif";
          else if(tmp=="PNG")                 mimeType="image/png";
          else if(tmp=="TIFF" || tmp=="TIF")  mimeType="image/tiff";
          else if(tmp=="XPM")                 mimeType="image/x-xpm";
          else if(tmp=="XBM")                 mimeType="image/x-xbm";
          else if(tmp=="BMP")                 mimeType="image/x-bmp";
          else if(tmp=="TXT" ||
                  tmp=="ASC" ||
                  tmp=="H" ||
                  tmp=="C" ||
                  tmp=="CC" ||
                  tmp=="CPP")                 mimeType="text/plain";
          else if(tmp=="HTML" || tmp=="HTM")  mimeType="text/html";
          else                                mimeType="application/octet-stream";
        }
      }
      m_imeTypes.append(mimeType);
      firstIteration=false;

      int next = s_rc.find('\n', endPos+1);
      if(next==-1) { //no more line breaks found, we give up
        success = false;
        break;
      } else
        next++; //points now at the beginning of the next line
      currentPos = next;

    } else {
      success = false;
    }
  }

  // append trailing text part of the article
  t_ext.append(s_rc.right(s_rc.length()-currentPos));

  return ((b_ins.count()>0) || isPartial());
}


}; // namespace Parser
}; // namespace KMime
