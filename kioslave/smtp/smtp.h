#ifndef _SMTP_H
#define _SMTP_H

#include <qstring.h>
#include <kio/slavebase.h>

#ifdef SSMTP
  #ifndef HAVE_SSL
    #undef SSMTP
  #endif
#endif  


#ifdef SSMTP
  extern "C" { 
  #include <openssl/crypto.h> 
  #include <openssl/x509.h> 
  #include <openssl/pem.h> 
  #include <openssl/ssl.h> 
  #include <openssl/err.h> 
  };
#endif                                                                                 

class SMTPProtocol : public KIO::SlaveBase {
 public:
  SMTPProtocol( const QCString &pool, const QCString &app );
  virtual ~SMTPProtocol();

 private:
#ifdef SSMTP
  SSL_CTX *ctx;
  SSL *ssl;
  X509 *server_cert;
  SSL_METHOD *meth;
#endif

};

#endif
