// The MIT License (MIT)
// Copyright (c) 2014 Yufei (Benny) Chen
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef LIBRARY_SYS_UTILS_H_
#define LIBRARY_SYS_UTILS_H_

#include <evhtp.h>
#include <iostream>
#include <unistd.h>
#include "../threadlocal.h"

namespace loopy {

typedef evhtp_request_t* pReq;

enum HTTP_STATUS_CODE {
  L_OK   = EVHTP_RES_200,
  L_300  = EVHTP_RES_300,
  L_400  = EVHTP_RES_400,
  L_NOT_FOUND = EVHTP_RES_NOTFOUND,
  L_SERVER_ERROR = EVHTP_RES_SERVERR,
  L_CONTINUE = EVHTP_RES_CONTINUE,
  L_FORBIDDEN = EVHTP_RES_FORBIDDEN,
  L_SWITCH_PROTO = EVHTP_RES_SWITCH_PROTO,
  L_MOVEDPERM = EVHTP_RES_MOVEDPERM,
  L_PROCESSING = EVHTP_RES_PROCESSING,
  L_URL_TOO_LONG = EVHTP_RES_URI_TOOLONG,
  L_CREATED = EVHTP_RES_CREATED,
  L_ACCEPTED = EVHTP_RES_ACCEPTED,
  L_NO_AUTH_INFO = EVHTP_RES_NAUTHINFO,
  L_NO_CONTENT = EVHTP_RES_NOCONTENT,
  L_RST_CONTENT = EVHTP_RES_RSTCONTENT,
  L_PARTIAL_CONTENT = EVHTP_RES_PARTIAL
// TODO: implement the rest
// case EVHTP_RES_MSTATUS:
// return "Multi-Status";
// case EVHTP_RES_IMUSED:
// return "IM Used";
// case EVHTP_RES_FOUND:
// return "Found";
// case EVHTP_RES_SEEOTHER:
// return "See Other";
// case EVHTP_RES_NOTMOD:
// return "Not Modified";
// case EVHTP_RES_USEPROXY:
// return "Use Proxy";
// case EVHTP_RES_SWITCHPROXY:
// return "Switch Proxy";
// case EVHTP_RES_TMPREDIR:
// return "Temporary Redirect";
// case EVHTP_RES_UNAUTH:
// return "Unauthorized";
// case EVHTP_RES_PAYREQ:
// return "Payment Required";
// case EVHTP_RES_METHNALLOWED:
// return "Not Allowed";
// case EVHTP_RES_NACCEPTABLE:
// return "Not Acceptable";
// case EVHTP_RES_PROXYAUTHREQ:
// return "Proxy Authentication Required";
// case EVHTP_RES_TIMEOUT:
// return "Request Timeout";
// case EVHTP_RES_CONFLICT:
// return "Conflict";
// case EVHTP_RES_GONE:
// return "Gone";
// case EVHTP_RES_LENREQ:
// return "Length Required";
// case EVHTP_RES_PRECONDFAIL:
// return "Precondition Failed";
// case EVHTP_RES_ENTOOLARGE:
// return "Entity Too Large";
// case EVHTP_RES_URITOOLARGE:
// return "Request-URI Too Long";
// case EVHTP_RES_UNSUPPORTED:
// return "Unsupported Media Type";
// case EVHTP_RES_RANGENOTSC:
// return "Requested Range Not Satisfiable";
// case EVHTP_RES_EXPECTFAIL:
// return "Expectation Failed";
// case EVHTP_RES_IAMATEAPOT:
// return "I'm a teapot";
// case EVHTP_RES_NOTIMPL:
// return "Not Implemented";
// case EVHTP_RES_BADGATEWAY:
// return "Bad Gateway";
// case EVHTP_RES_SERVUNAVAIL:
// return "Service Unavailable";
// case EVHTP_RES_GWTIMEOUT:
// return "Gateway Timeout";
// case EVHTP_RES_VERNSUPPORT:
// return "HTTP Version Not Supported";
// case EVHTP_RES_BWEXEED:
// return "Bandwidth Limit Exceeded";
};
const char* getMethodName(htp_method method);
evthr_t* getRequestThread(evhtp_request_t * request);

// helper template to decide if the two types are the same
template <class T1, class T2> struct SameType {
  enum{value = false};
};
template <class T> struct SameType<T, T> {
  enum{value = true};
};

// helper function to make and free dummy connections
void free_dummy_conn(evhtp_connection_t* conn);
evhtp_connection_t * new_dummy_conn(evhtp_t* htp);

void dummyInitializeThread(evthr_t * thread, void* arg);

// helper function to make and free dummy requests
template<typename T>
evhtp_request_t * new_dummy_request() {

  evbase_t * evbase = event_base_new();
  evhtp_t  * htp    = evhtp_new(evbase, NULL);

  evhtp_request_t * req;
  
  evhtp_connection_t* conn = new_dummy_conn(htp);
  conn->thread = evthr_new(dummyInitializeThread, (void*)&T::initThread);
  int result = evthr_start(conn->thread);
  // FIXME: it seems to me that the compiler reordered the place where 
  // pthread_create is called, so yield control here and trick the OS into
  // creating the thread. Not the right way of doing things. But why does
  // the callback for pthread_create not get called
  std::cout << "Sleeping to wait for the thread. FIXME!!" << std::endl;
  sleep(1);

  evhtp_uri_t * uri = new evhtp_uri_t;
  uri->path = new evhtp_path_t;
  uri->path->full = "/";
  uri->path->file = "/";
  uri->path->path = "/";

  if (!(req = new evhtp_request_t)) {
    return nullptr;
  }
  
  req->conn = conn;
  req->htp = htp;
  req->uri = uri;
  req->status = EVHTP_RES_OK;
  req->buffer_in = evbuffer_new();
  req->buffer_out = evbuffer_new();
  req->headers_in = new evhtp_headers_t;
  req->headers_out = new evhtp_headers_t;
  TAILQ_INIT(req->headers_in);
  TAILQ_INIT(req->headers_out);
  return req;
}
void free_dummy_request(evhtp_request_t* req);

}  // namespace loopy

#endif  // LIBRARY_SYS_UTILS_H_
