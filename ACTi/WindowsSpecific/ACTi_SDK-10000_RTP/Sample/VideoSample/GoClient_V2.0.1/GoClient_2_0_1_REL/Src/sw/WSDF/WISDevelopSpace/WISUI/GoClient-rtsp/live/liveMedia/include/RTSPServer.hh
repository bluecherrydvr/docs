/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2004 Live Networks, Inc.  All rights reserved.
// A RTSP server
// C++ header

#ifndef _RTSP_SERVER_HH
#define _RTSP_SERVER_HH

#ifndef _SERVER_MEDIA_SESSION_HH
#include "ServerMediaSession.hh"
#endif
#ifndef _NET_ADDRESS_HH
#include <NetAddress.hh>
#endif
#ifndef _DIGEST_AUTHENTICATION_HH
#include "DigestAuthentication.hh"
#endif

// A data structure used for optional user/password authentication:

class UserAuthenticationDatabase {
public:
  UserAuthenticationDatabase(char const* realm = NULL,
			     Boolean passwordsAreMD5 = False);
    // If "passwordsAreMD5" is True, then each password stored into, or removed from,
    // the database is actually the value computed
    // by md5(<username>:<realm>:<actual-password>)
  virtual ~UserAuthenticationDatabase();

  virtual void addUserRecord(char const* username, char const* password);
  virtual void removeUserRecord(char const* username);

  virtual char const* lookupPassword(char const* username);
      // returns NULL if the user name was not present

  char const* realm() { return fRealm; }
  Boolean passwordsAreMD5() { return fPasswordsAreMD5; }

protected:
  HashTable* fTable;
  char* fRealm;
  Boolean fPasswordsAreMD5;
};

class RTSPServer: public Medium {
public:
  static RTSPServer* createNew(UsageEnvironment& env, Port ourPort = 554,
			       UserAuthenticationDatabase* authDatabase = NULL);
      // if ourPort.num() == 0, we'll choose the port number
      // Note: The caller is responsible for reclaiming "authDatabase"

  static Boolean lookupByName(UsageEnvironment& env, char const* name,
			      RTSPServer*& resultServer);

  void addServerMediaSession(ServerMediaSession* serverMediaSession);
  void removeServerMediaSession(ServerMediaSession* serverMediaSession);

  char* rtspURL(ServerMediaSession const* serverMediaSession) const;
      // returns a "rtsp://" URL that could be used to access the
      // specified session (which must already have been added to
      // us using "addServerMediaSession()".
      // This string is dynamically allocated; caller should delete[]

protected:
  RTSPServer(UsageEnvironment& env,
	     int ourSocket, Port ourPort,
	     UserAuthenticationDatabase* authDatabase);
      // called only by createNew();
  virtual ~RTSPServer();

  static int setUpOurSocket(UsageEnvironment& env, Port& ourPort);

private: // redefined virtual functions
  virtual Boolean isRTSPServer() const;

private:
  static void incomingConnectionHandler(void*, int /*mask*/);
  void incomingConnectionHandler1();

  // The state of each individual session handled by a RTSP server:
  class RTSPClientSession {
  public:
    RTSPClientSession(RTSPServer& ourServer, unsigned sessionId,
		      int clientSocket, struct sockaddr_in clientAddr);
    virtual ~RTSPClientSession();
  private:
    UsageEnvironment& envir() { return fOurServer.envir(); }
    void reclaimStreamStates();
    static void incomingRequestHandler(void*, int /*mask*/);
    void incomingRequestHandler1();
    void handleCmd_bad(char const* cseq);
    void handleCmd_notSupported(char const* cseq);
    void handleCmd_notFound(char const* cseq);
    void handleCmd_unsupportedTransport(char const* cseq);
    void handleCmd_OPTIONS(char const* cseq);
    void handleCmd_DESCRIBE(char const* cseq, char const* urlSuffix,
			    char const* fullRequestStr);
    void handleCmd_SETUP(char const* cseq,
			 char const* urlPreSuffix, char const* urlSuffix,
			 char const* fullRequestStr);
    void handleCmd_withinSession(char const* cmdName,
				 char const* urlPreSuffix, char const* urlSuffix,
				 char const* cseq, char const* fullRequestStr);
    void handleCmd_TEARDOWN(ServerMediaSubsession* subsession,
			    char const* cseq);
    void handleCmd_PLAY(ServerMediaSubsession* subsession,
			char const* cseq, char const* fullRequestStr);
    void handleCmd_PAUSE(ServerMediaSubsession* subsession,
			 char const* cseq);
    Boolean authenticationOK(char const* cmdName, char const* cseq,
			     char const* fullRequestStr);
    Boolean parseRequestString(char const *reqStr, unsigned reqStrSize,
			       char *resultCmdName,
			       unsigned resultCmdNameMaxSize, 
			       char* resultURLPreSuffix,
			       unsigned resultURLPreSuffixMaxSize, 
			       char* resultURLSuffix,
			       unsigned resultURLSuffixMaxSize, 
			       char* resultCSeq,
			       unsigned resultCSeqMaxSize); 

  private:
    RTSPServer& fOurServer;
    unsigned fOurSessionId;
    ServerMediaSession* fOurServerMediaSession;
    int fClientSocket;
    struct sockaddr_in fClientAddr;
    unsigned char fBuffer[10000];
    Boolean fSessionIsActive;
    Authenticator fCurrentAuthenticator; // used if access control is needed
    unsigned fNumStreamStates; 
    struct streamState {
      ServerMediaSubsession* subsession;
      void* streamToken;
    } * fStreamStates;
  };

private:
  friend class RTSPClientSession;
  int fServerSocket;
  Port fServerPort;
  UserAuthenticationDatabase* fAuthDB;
  HashTable* fServerMediaSessions;
  unsigned fSessionIdCounter;
};

#endif