// $Id$

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <qcstring.h>
#include <qglobal.h>

#include <kurl.h>
#include <kprotocolmanager.h>
#include <ksock.h>
#include <kio_interface.h>
#include <kinstance.h>

#include "nntp.h"

bool open_PassDlg( const QString& _head, QString& _user, QString& _pass );

int main(int , char **)
{
  signal(SIGCHLD, KIOProtocol::sigchld_handler);
// Prevents coredumps (from SIGSEGV) if -DNDEBUG is used
#ifdef NDEBUG
  signal(SIGSEGV, KIOProtocol::sigsegv_handler);
#endif

  KInstance instance( "kio_nntp" );

  KIOConnection parent( 0, 1 );

  NNTPProtocol nntp( &parent );
  nntp.dispatchLoop();
}

NNTPProtocol::NNTPProtocol(KIOConnection *_conn) : KIOProtocol(_conn)
{
  m_cmd = CMD_NONE;
  m_pJob = 0L;
  m_iSock = m_iOldPort = 0;
  m_tTimeout.tv_sec=10;
  m_tTimeout.tv_usec=0;
  fp = 0;
}

bool NNTPProtocol::getResponse (char *r_buf, unsigned int r_len)
{
  char *buf=0;
  unsigned int recv_len=0;
  fd_set FDs;

  // Give the buffer the appropiate size
  if (r_len)
    buf=(char *)malloc(r_len);
  else {
    buf=(char *)malloc(512);
    r_len=512;
  }

  // And keep waiting if it timed out
  unsigned int wait_time=60; // Wait 60sec. max.
  do
  {
    // Wait for something to come from the server
    FD_ZERO(&FDs);
    FD_SET(m_iSock, &FDs);
    // Yes, it's true, Linux sucks. (And you can't program --Waba)
    wait_time--;
    m_tTimeout.tv_sec=1;
    m_tTimeout.tv_usec=0;
  }
  while (wait_time && (::select(m_iSock+1, &FDs, 0, 0, &m_tTimeout) ==0));

  if (wait_time == 0)
  {
    fprintf(stderr, "No response from NNTP server in 60 secs.\n");fflush(stderr);
    return false;
  }

  // Clear out the buffer
  memset(buf, 0, r_len);
  // And grab the data
  if (fgets(buf, r_len-1, fp) == 0) {
    if (buf) free(buf);
    return false;
  }
  // This is really a funky crash waiting to happen if something isn't
  // null terminated.
  recv_len=strlen(buf);

/*
 *   From rfc1939:
 *
 *   Responses in the NNTP consist of a status indicator and a keyword
 *   possibly followed by additional information.  All responses are
 *   terminated by a CRLF pair.  Responses may be up to 512 characters
 *   long, including the terminating CRLF.  There are currently two status
 *   indicators: positive ("+OK") and negative ("-ERR").  Servers MUST
 *   send the "+OK" and "-ERR" in upper case.
 */
	fprintf(stderr,"%s\n",buf);
/*  if ((strncmp(buf, "200", 3)==0)||(strncmp(buf,"201",3)==0)||(strncmp(buf,"381",3)==0)) { */
    if (!(strncmp(buf, "501",3)==0)||!(strncmp(buf,"502",3)==0))
 {
    if (r_buf && r_len) {
      memcpy(r_buf, buf+3, QMIN(r_len,recv_len-4));
    }
    if (buf) free(buf);
    return true;
  } else {
    if (r_buf && r_len) {
      memcpy(r_buf, buf+3, QMIN(r_len,recv_len-3));
    }
    if (buf) free(buf);
    return false;
  }
}

bool NNTPProtocol::command (const char *cmd, char *recv_buf, unsigned int len)
{

/*
 *   From rfc1939:
 *
 *   Commands in the NNTP consist of a case-insensitive keyword, possibly
 *   followed by one or more arguments.  All commands are terminated by a
 *   CRLF pair.  Keywords and arguments consist of printable ASCII
 *   characters.  Keywords and arguments are each separated by a single
 *   SPACE character.  Keywords are three or four characters long. Each
 *   argument may be up to 40 characters long.
 */

  fprintf(stderr, "%s\n",cmd);
  // Write the command
  if (::write(m_iSock, cmd, strlen(cmd)) != (ssize_t)strlen(cmd))
    return false;
  if (::write(m_iSock, "\r\n", 2) != 2)
    return false;
  return getResponse(recv_buf, len);
}

void NNTPProtocol::nntp_close ()
{
  // If the file pointer exists, we can assume the socket is valid,
  // and to make sure that the server doesn't magically undo any of
  // our deletions and so-on, we should send a QUIT and wait for a
  // response.  We don't care if it's positive or negative.  Also
  // flush out any semblance of a persistant connection, i.e.: the
  // old username and password are now invalid.
  if (fp) {
    (void)command("QUIT");
    fclose(fp);
    m_iSock=0; fp=0;
    m_sOldUser = ""; m_sOldPass = ""; m_sOldServer = "";
  }
}

bool NNTPProtocol::nntp_open( KURL &_url )
{
  // This function is simply a wrapper to establish the connection
  // to the server.  It's a bit more complicated than ::connect
  // because we first have to check to see if the user specified
  // a port, and if so use it, otherwise we check to see if there
  // is a port specified in /etc/services, and if so use that
  // otherwise as a last resort use the "official" port of 119.

  unsigned short int port;
  ksockaddr_in server_name;
  memset(&server_name, 0, sizeof(server_name));
  static char buf[512];

  // We want 119 as the default, but -1 means no port was specified.
  // Why 0 wasn't chosen is beyond me.
  port = _url.port() ? _url.port() : 119;
  if ( (m_iOldPort == port) && (m_sOldServer == _url.host()) && (m_sOldUser == _url.user()) && (m_sOldPass == _url.pass())) {
    fprintf(stderr,"Reusing old connection\n");fflush(stderr);
    return true;
  } else {
    nntp_close();
    m_iSock = ::socket(PF_INET, SOCK_STREAM, 0);
    if (!KSocket::initSockaddr(&server_name, _url.host(), port))
      return false;
    if (::connect(m_iSock, (struct sockaddr*)(&server_name), sizeof(server_name))) {
      error( ERR_COULD_NOT_CONNECT, strdup(_url.host()));
      return false;
    }

    // Since we want to use stdio on the socket,
    // we must fdopen it to get a file pointer,
    // if it fails, close everything up
    if ((fp = fdopen(m_iSock, "w+")) == 0) {
      close(m_iSock);
      return false;
    }

    if (!getResponse())  // If the server doesn't respond with a greeting
      return false;      // we've got major problems, and possibly the
                         // wrong port

    m_iOldPort = port;
    m_sOldServer = _url.host();

   memset(buf, 0, sizeof(buf));
   QString usr, pass, one_string="MODE READER";
   if (!command(one_string, buf, sizeof(buf))) {
	fprintf(stderr,"This is an error message\n"); fflush(stderr);
	}
    

    one_string="AUTHINFO USER ";
    if (_url.user().isEmpty() || _url.pass().isEmpty()) {
      // Prompt for usernames
      QString head="Username and password for your NNTP account:";
      if (!open_PassDlg(head, usr, pass)) {
	return false;
	nntp_close();
      } else {
	one_string.append(usr);
	m_sOldUser=usr;
      }
    } else {
      one_string.append(_url.user());
      m_sOldUser = _url.user();
    }

    memset(buf, 0, sizeof(buf));
    if (!command(one_string, buf, sizeof(buf))) {
      fprintf(stderr, "Couldn't login. Bad username Sorry\n"); fflush(stderr);
      nntp_close();
      return false;
    }
    
    one_string="AUTHINFO PASS ";
    if (_url.pass().isEmpty()) {
      m_sOldPass = pass;
      one_string.append(pass);
    } else {
      m_sOldPass = _url.pass();
      one_string.append(_url.pass());
    }
    if (!command(one_string, buf, sizeof(buf))) {
      fprintf(stderr, "Couldn't login. Bad password Sorry\n"); fflush(stderr);
      nntp_close();
      return false;
    }
    return true;

  }
}

size_t NNTPProtocol::realGetSize(unsigned int msg_num)
{
  char *buf;
  QCString cmd;
  size_t ret=0;
  buf=(char *)malloc(512);
  memset(buf, 0, 512);
  cmd.sprintf("LIST %u", msg_num);
  if (!command(cmd.data(), buf, 512)) {
    free(buf);
    return 0;
  } else {
    cmd=buf;
    cmd.remove(0, cmd.find(" "));
    ret=cmd.toLong();
  }
  free(buf);
  return ret;
}

void NNTPProtocol::slotGetSize( const char * _url )
{
  // This should I deally call the totalSize function for the URL,
  // but I haven't really tested this.
  // bool ok=true;
  // static char buf[512];
  QString path, cmd;
  KURL usrc(_url);

  // We can't work on an invalid URL.
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if (usrc.protocol() != "news") {
    error( ERR_INTERNAL, "kio_nntp got non nntp url" );
    m_cmd = CMD_NONE;
    return;
  }

  if (path.at(0)=='/') path.remove(0,1);
  if (path.isEmpty()) {
    qDebug("We should be a dir!!");
    error(ERR_IS_DIRECTORY, strdup(_url));
    m_cmd=CMD_NONE; return;
  }
  if (path.left(8)=="Message ") path.remove(0,8);

  bool isINT;
  int msg_num=path.toUInt(&isINT);
  if (!isINT) {
    //error(ERR_MALFORMED_URL, strdup(_url));
    return;
  }

  if (!nntp_open(usrc)) {
    fprintf(stderr,"nntp_open failed\n");fflush(stderr);
    nntp_close();
    return;
  }

  totalSize(realGetSize(msg_num));
  finished();
  m_cmd = CMD_NONE;

}


void NNTPProtocol::slotGet(const char *_url)
{
// List of supported commands
//
// URI                                Command   Result
// news://user:pass@domain/index       LIST      List newsgroups
// news://user:pass@domain/group/name     GROUP NAME      change to NAME
// news://user:pass@domain/remove/#1   DELE #1   Mark a message for deletion
// news://user:pass@domain/get/MSGID RETR MSGID   Get message header and body
// news://user:pass@domain/list/#1     LIST #1   Get size of a message
// news://user:pass@domain/uid/#1      UIDL #1   Get UID of a message
// news://user:pass@domain/commit      QUIT      Delete marked messages
// news://user:pass@domain/headers/#1  TOP #1    Get header of message
//
// Notes:
// Sizes are in bytes.
// No support for the STAT command has been implemented.
// commit closes the connection to the server after issuing the QUIT command.

  bool ok=true;
  static char buf[512];
  QString path, cmd;
  KURL usrc(_url);
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if (usrc.protocol() != "news") {
    error( ERR_INTERNAL, "kio_nntp got non nntp url" );
    m_cmd = CMD_NONE;
    return;
  }

  path = usrc.path().copy();

  if (path.at(0)=='/') path.remove(0,1);
  if (path.isEmpty()) {
    debug("We should be a dir!!");
    error(ERR_IS_DIRECTORY, strdup(_url));
    m_cmd=CMD_NONE; return;
  }

  if (((path.find("/") == -1) && (path != "index") && 
       (path != "uidl") && (path != "commit")) ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return; 
  }

  cmd = path.left(path.find("/"));
  path.remove(0,path.find("/")+1);

  if (!nntp_open(usrc)) {
    fprintf(stderr,"nntp_open failed\n");fflush(stderr);
    nntp_close();
    return;
  }

  if ((cmd == "index") || (cmd == "uidl")) {
    unsigned long size=0;
    bool result;
    if (cmd == "index")
      result = command("LIST");
    else
      result = command("UIDL");
    if (result) {
      ready();
      gettingFile(_url);
      while (!feof(fp)) {
	memset(buf, 0, sizeof(buf));
	if (!fgets(buf, sizeof(buf)-1, fp))
	  break;  // Error??
	// HACK: This assumes fread stops at the first \n and not \r
	buf[strlen(buf)-2]='\0';
	if (strcmp(buf, ".")==0)  break; // End of data.	
	else {
		size+=strlen(buf);
		data(buf, strlen(buf));
		totalSize(size);
	}
      }
      fprintf(stderr,"Finishing up list\n");fflush(stderr);
      dataEnd();
      speed(0); finished();
    }
  }

  else if (cmd == "headers") {
    (void)path.toInt(&ok);
    if (!ok) return; //  We fscking need a number!
    path.prepend("TOP ");
    path.append(" 0");
    if (command(path)) { // This should be checked, and a more hackish way of
                         // getting at the headers by d/l the whole message
                         // and stopping at the first blank line used if the
                         // TOP cmd isn't supported
      ready();
      gettingFile(_url);
      mimeType("text/plain");
      memset(buf, 0, sizeof(buf));
      while (!feof(fp)) {
	memset(buf, 0, sizeof(buf));
	if (!fgets(buf, sizeof(buf)-1, fp))
	  break;  // Error??
	// HACK: This assumes fread stops at the first \n and not \r
	buf[strlen(buf)-2]='\0';
	if (strcmp(buf, ".")==0)  break; // End of data.
	else if (strcmp(buf, "..")==0)
		data (".", 1);
	else
		data(buf, strlen(buf));
      }
      fprintf(stderr,"Finishing up\n");fflush(stderr);
      dataEnd();
      speed(0); finished();
    }
  }

else if (cmd == "get") {
    //(void)path.toInt(&ok);
    //if (!ok) return; //  We fscking need a number!
    path.prepend("ARTICLE ");
    //path.append(" 0");
    if (command(path)) { // This should be checked, and a more hackish way of
                         // getting at the headers by d/l the whole message
                         // and stopping at the first blank line used if the
                         // TOP cmd isn't supported
      ready();
      gettingFile(_url);
      mimeType("text/plain");
      memset(buf, 0, sizeof(buf));
      while (!feof(fp)) {
	memset(buf, 0, sizeof(buf));
	if (!fgets(buf, sizeof(buf)-1, fp))
	  break;  // Error??
	// HACK: This assumes fread stops at the first \n and not \r
	buf[strlen(buf)-2]='\0';
	if (strcmp(buf, ".")==0)  break; // End of data.
	else 
	if (strcmp(buf, "..")==0)
		data (".", 1);
	else	
		{
		fprintf(stderr,"%s\n",buf);
		data(buf, strlen(buf));
		}
      }
      fprintf(stderr,"Finishing up\n");fflush(stderr);
      dataEnd();
      speed(0); finished();
    }
  }

else if (cmd == "group") {
    (void)path.toInt(&ok);
   // if (!ok) return; //  We fscking need a number!
    path.prepend("GROUP ");
    if (command(path)) { // This should be checked, and a more hackish way of
                         // getting at the headers by d/l the whole message
                         // and stopping at the first blank line used if the
                         // TOP cmd isn't supported
      ready();
      gettingFile(_url);
      mimeType("text/plain");
      memset(buf, 0, sizeof(buf));
      while (!feof(fp)) {
	memset(buf, 0, sizeof(buf));
	if (!fgets(buf, sizeof(buf)-1, fp))
	  break;  // Error??
	// HACK: This assumes fread stops at the first \n and not \r
	buf[strlen(buf)-2]='\0';
	if (strcmp(buf, ".")==0)  break; // End of data.
	else if (strcmp(buf, "..")==0)
		data (".", 1);
	else
		data(buf, strlen(buf));
      }
      fprintf(stderr,"Finishing up\n");fflush(stderr);
      dataEnd();
      speed(0); finished();
    }
  }




  else if (cmd == "remove") {
    (void)path.toInt(&ok);
    if (!ok) return; //  We fscking need a number!
    path.prepend("DELE ");
    command(path);
    finished();
    m_cmd = CMD_NONE;
  }
  
  else if (cmd == "download") {
    int p_size=0;
    unsigned int msg_len=0;
    (void)path.toInt(&ok);
    QString list_cmd("LIST ");
    if (!ok)
      return; //  We fscking need a number!
    list_cmd+= path;
    path.prepend("RETR ");
    memset(buf, 0, sizeof(buf));
    if (command(list_cmd, buf, sizeof(buf)-1)) {
      list_cmd=buf;
      // We need a space, otherwise we got an invalid reply
      if (!list_cmd.find(" ")) {
	debug("List command needs a space? %s", list_cmd.data());
        nntp_close();
        return;
      }
      list_cmd.remove(0, list_cmd.find(" ")+1);
      msg_len = list_cmd.toUInt(&ok);
      if (!ok) {
	debug("LIST command needs to return a number? :%s:", list_cmd.data());
	nntp_close();return;
      }
    } else {
      nntp_close(); return;
    }
    if (command(path)) {
      ready();
      gettingFile(_url);
      mimeType("message/rfc822");
      totalSize(msg_len);
      memset(buf, 0, sizeof(buf));
      while (!feof(fp)) {
	memset(buf, 0, sizeof(buf));
	if (!fgets(buf, sizeof(buf)-1, fp))
	  break;  // Error??
	// HACK: This assumes fread stops at the first \n and not \r
	buf[strlen(buf)-2]='\0';
	if (strcmp(buf, ".")==0)  break; // End of data.
	data(buf, strlen(buf));
	p_size+=strlen(buf);
	processedSize(p_size);
      }
      fprintf(stderr,"Finishing up\n");fflush(stderr);
      dataEnd();
      speed(0); finished();
    } else {
      fprintf(stderr, "Couldn't login. Bad RETR Sorry\n");
      fflush(stderr);
      nntp_close();
      return;
    }
  }

  else if ((cmd == "uid") || (cmd == "list")) {
    QString qbuf;
    (void)path.toInt(&ok);
    if (!ok) return; //  We fscking need a number!
    if (cmd == "uid")
      path.prepend("UIDL ");
    else
      path.prepend("LIST ");
    memset(buf, 0, sizeof(buf));
    if (command(path, buf, sizeof(buf)-1)) {
      const int len = strlen(buf);
      ready();
      gettingFile(_url);
      mimeType("text/plain");
      totalSize(len);
      data(buf, len);
      processedSize(len);
      debug( buf );
      fprintf(stderr,"Finishing up uid\n");fflush(stderr);
      dataEnd();
      speed(0); finished();
    } else {
      nntp_close(); return;
    }
  }

  else if (cmd == "commit") {
    nntp_close();
    finished();
    m_cmd = CMD_NONE;
    return;
  }
// New message get command starts here
else {
  cmd.prepend("GROUP ");   
  command(cmd);  
   path.prepend("ARTICLE ");
    if (command(path)) { // This should be checked, and a more hackish way of
                         // getting at the headers by d/l the whole message
                         // and stopping at the first blank line used if the
                         // TOP cmd isn't supported
      ready();
      gettingFile(_url);
      mimeType("text/plain");
      memset(buf, 0, sizeof(buf));
      while (!feof(fp)) {
	memset(buf, 0, sizeof(buf));
	if (!fgets(buf, sizeof(buf)-1, fp))
	  break;  // Error??
	// HACK: This assumes fread stops at the first \n and not \r
	buf[strlen(buf)-2]='\0';
	if (strcmp(buf, ".")==0)  break; // End of data.
	else if (strcmp(buf, "..")==0)
		data (".", 1);
	else
		data(buf, strlen(buf));
      }
      fprintf(stderr,"Finishing up\n");fflush(stderr);
      dataEnd();
      speed(0); finished();
    }
  }


// New message get ends here - CJM
}

void NNTPProtocol::slotListDir (const char *_url)
{

  char buf[512];
  QCString q_buf;
  KURL usrc( _url );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, _url );
    return;
  }
  // Try and open a connection
  if (!nntp_open(usrc)) {
    fprintf(stderr,"nntp_open failed\n");fflush(stderr);
    nntp_close();
    return;
  }
  // Check how many messages we have. STAT is by law required to
  // at least return +OK num_messages total_size
  memset(buf, 0, 512);
  if (!command("LIST", buf, sizeof(buf))) {
    error(ERR_INTERNAL, "??");
    return;
  } 
  finished();
}

void NNTPProtocol::slotTestDir (const char *_url)
{
	KURL usrc(_url);
	// An empty path is essentially a request for an index...
	if (usrc.path() == "/" || usrc.path() == "")
		isDirectory();
	else
		isFile();
	finished();
}

void NNTPProtocol::slotCopy(const char *, const char *)
{
  fprintf(stderr, "NNTPProtocol::slotCopy\n");
  fflush(stderr);
}

void NNTPProtocol::slotData(void *, int)
{
  switch (m_cmd) {
    case CMD_PUT:
	// Send data here
      break;
    default:
      abort();
      break;
    }
}

void NNTPProtocol::slotDataEnd()
{
  switch (m_cmd) {
    case CMD_PUT:
      m_cmd = CMD_NONE;
      break;
    default:
      abort();
      break;
    }
}

void NNTPProtocol::jobData(void *, int )
{
  switch (m_cmd) {
  case CMD_GET:
    break;
  case CMD_COPY:
    break;
  default:
    abort();
  }
}

void NNTPProtocol::jobError(int _errid, const char *_text)
{
  error(_errid, _text);
}

void NNTPProtocol::jobDataEnd()
{
  switch (m_cmd) {
  case CMD_GET:
    dataEnd();
    break;
  case CMD_COPY:
    m_pJob->dataEnd();
    break;
  default:
    abort();
  }
}

void NNTPProtocol::slotDel( QStringList& _list )
{
    // FIXME: this should iterate through the possible URIs, and
    // put them into some sort of dict based on the server they correspond
    // to, and count them and validate them.  *then* and only then
    // should it actually try and delete them.

    QStringList::Iterator files = _list.begin();
    QString path, invalidURI=QString::null;
    bool isInt;
    KURL blah=(*_list.begin());
    if ( !nntp_open(blah) ) {
      fprintf(stderr,"nntp_open failed\n");fflush(stderr);
      nntp_close();
      return;
    }
    totalSize(_list.count());
    totalFiles(_list.count());
    totalDirs(0);
    for (; files != _list.end(); ++files) {
	KURL target(*files);
	if ( target.isMalformed() ) {
	  error( ERR_MALFORMED_URL, *files );
	  m_cmd = CMD_NONE;
	  return;
	}
	path=target.path();
	if (path.at(0) == '/')
	  path.remove(0,1);
	(void)path.toUInt(&isInt);
	if (!isInt) {
	  invalidURI=path;
	} else {
	  path.prepend("DELE ");
	  if (!command(path)) {
	    invalidURI=path;
	  }
	}
    }
    if (!invalidURI.isEmpty()) {
	error(ERR_MALFORMED_URL, invalidURI);
    }
    nntp_close();
    finished();
    m_cmd=CMD_NONE;
}

/*************************************
 *
 * NNTPIOJob
 *
 *************************************/

NNTPIOJob::NNTPIOJob(KIOConnection *_conn, NNTPProtocol *_nntp) :
	KIOJobBase(_conn)
{
  m_pNNTP = _nntp;
}
  
void NNTPIOJob::slotData(void *_p, int _len)
{
  m_pNNTP->jobData( _p, _len );
}

void NNTPIOJob::slotDataEnd()
{
  m_pNNTP->jobDataEnd();
}

void NNTPIOJob::slotError(int _errid, const char *_txt)
{
  KIOJobBase::slotError( _errid, _txt );
  m_pNNTP->jobError(_errid, _txt );
}
