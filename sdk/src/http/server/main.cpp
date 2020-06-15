// Copyright 2020, OpenTelemetry Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>

// Debug macros
#define HAVE_CONSOLE_LOG
#define HAVE_HTTP_DEBUG

// Macro that allows to implement HTTP server in any namespace
#define HTTP_SERVER_NS testing

namespace HTTP_SERVER_NS {};

#include "HttpServer.hpp"

using namespace HTTP_SERVER_NS;

/// <summary>
/// Example HTTP server at http://127.0.0.1:32000/
/// </summary>
class ExampleHttpServer : public HttpServer::Callback
{
protected:

  HttpServer _server;

  int _port{32000};

  std::string _hostname{"localhost"};

  enum RequestState
  {
    Planned,
    Sent,
    Processed,
    Done
  };

  std::atomic<uint32_t> reqCount{0};
  std::mutex _lock;

public:

  ExampleHttpServer(){};

  /// <summary>
  /// CLear all responses
  /// </summary>
  void Clear()
  {

  }

  /// <summary>
  /// Set up simple server
  /// </summary>
  virtual void SetUp() /* override */
  {
    _port = _server.addListeningPort(_port); /* 0 for random port number */
    std::ostringstream os;
    os << "localhost:" << _port;
    _hostname = os.str();
    _server.setServerName(_hostname);
    _server.addHandler("/simple/", *this);
    _server.addHandler("/echo/", *this);
    _server.addHandler("/count/", *this);
    _server.start();
    Clear();
  }

  /// <summary>
  /// Teardown server
  /// </summary>
  virtual void TearDown() /* override */
  {
    _server.stop();
    Clear();
  }

protected:

  /// <summary>
  /// HTTP request handler
  /// </summary>
  /// <param name="request"></param>
  /// <param name="inResponse"></param>
  /// <returns></returns>
  virtual int onHttpRequest(HttpServer::Request const &req,
                            HttpServer::Response &resp) override
  {
    // Simple plain text response
    if (req.uri.substr(0, 8) == "/simple/")
    {
      resp.headers["Content-Type"] = "text/plain";
      resp.content                 = "It works!";
      return std::atoi(req.uri.substr(8).c_str());
    }

    // Echo back the contents of what we received in HTTP POST
    if (req.uri == "/echo/")
    {
      auto it = req.headers.find("Content-Type");
      resp.headers["Content-Type"] =
          (it != req.headers.end()) ? it->second : "application/octet-stream";
      resp.content = req.content;
      return 200;
    }

    // Simple counter that tracks requests processed
    if (req.uri.substr(0, 7) == "/count/")
    {
      reqCount++;
      resp.headers["Content-Type"] = "text/plain";
      resp.message                 = "200 OK";
      resp.content                 = "Requests processed: ";
      resp.content += std::to_string(reqCount);
      return 200;
    }

    // Default handler
    resp.headers["Content-Type"] = "text/plain";
    resp.content                 = "Not Found";
    resp.code                    = 404;
    resp.message                 = "Not Found";
    return 404;

  }
};

int main()
{
  ExampleHttpServer server;
  std::cout << "Presss <ENTER> to stop.\n";
  server.SetUp();
  std::cin.get();
}
