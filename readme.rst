Purpose
=======
Stream any number of video/image sequences to any number of browser windows,
using opencv and boost::asio.
This could be used as the basis for a web browser based video stream viewer, powered by
opencv. Currently using ``multipart/x-mixed-replace`` of jpeg images.
See http://en.wikipedia.org/wiki/Motion_JPEG
Future goals include more sophisticated video streams, e.g. webm.

Dependencies
^^^^^^^^^^^^
* boost >= 1.40
* opencv >= 2.2
* cmake >= 2.8
* *optional* ecto https://github.com/plasmodic/ecto

build it
^^^^^^^^
::

  % git clone git://github.com/ethanrublee/streamer.git
  % cd streamer
  % mkdir build
  % cd build
  % cmake ..
  % make

try it
^^^^^^
::

  % cd streamer
  % build/bin/httpserv 0.0.0.0 9090 1 .
  Visit:
  0.0.0.0:9090/stream_0

What
^^^^
Server is based off of
http://www.boost.org/doc/libs/1_40_0/doc/html/boost_asio/examples.html#boost_asio.examples.http_server_3

Optional ecto cells and python bindings. https://github.com/plasmodic/ecto

main.cpp shows the server's use, generates a random color and streams it.

dir_reader.py demoes the servers use from ecto. see module.cpp for an ecto cell that uses the server.

http interface
==============
The a GET or POST to the url ``/_all`` is used for listing all available streams, as a text file with each line containing a url path.::
  
  % curl -X GET http://localhost:9090/_all
  /foobar
  /stream_0
  /stream_1

Also supports a listing based on a regular expression, where the base expression is
``_all<CUSTOM_REGEX>(.*)``, e.g.::

  % curl -X GET http://localhost:9090/_all/stream_
  /stream_0
  /stream_1
  % curl -X GET "http://localhost:9090/_all/(.*)bar"
  /foobar

POST may also be used::

  %curl -X POST "http://localhost:9090/_all/(.*)bar"
  /foobar

A GET of a ``/foobar`` will recieve a 302 redirect::

  % curl -vX GET "http://localhost:9090/foobar"
  * About to connect() to localhost port 9090 (#0)
  *   Trying 127.0.0.1... connected
  * Connected to localhost (127.0.0.1) port 9090 (#0)
  > GET /foobar HTTP/1.1
  > User-Agent: curl/7.21.3 (x86_64-pc-linux-gnu) libcurl/7.21.3 OpenSSL/0.9.8o zlib/1.2.3.4 libidn/1.18
  > Host: localhost:9090
  > Accept: */*
  > 
  * HTTP 1.0, assume close after body
  < HTTP/1.0 302 Moved Temporarily
  < Location: _stream/foobar/1303455736
  < 
  * Closing connection #0

This redirect is meant to fool the browser into not caching the connection, and returns a unique URL per ``GET``.
Each stream is found at ``/_stream/<STREAM_PATH>/(.*)``::

  % curl -vX GET "http://localhost:9090/_stream/foobar/12323" > /dev/null
  * About to connect() to localhost port 9090 (#0)
  *   Trying 127.0.0.1... connected
  * Connected to localhost (127.0.0.1) port 9090 (#0)
  > GET /_stream/foobar/12323 HTTP/1.1
  > User-Agent: curl/7.21.3 (x86_64-pc-linux-gnu) libcurl/7.21.3 OpenSSL/0.9.8o zlib/1.2.3.4 libidn/1.18
  > Host: localhost:9090
  > Accept: */*
  > 
  * HTTP 1.0, assume close after body
  < HTTP/1.0 200 OK
  < Connection: close
  < Max-Age: 0
  < Expires: 0
  < Cache-Control: no-cache
  < Pragma: no-cache
  < Content-Type: multipart/x-mixed-replace; boundary=--BOUNDARYSTRING
  < 
  { [data not shown]

The server will also return any file that is relative to where the document
root given at startup, including auto resolving ``index.html``...::

  % curl -X GET "http://localhost:9090"
  <html>
  <head></head>
  <body>
    <h1>An http mjpeg streaming server based on boost::asio</h1>
    <a href="/_all">/_all</a> a listing of all streams.
    <br />
    <img src="/stream_0" alt="A streaming jpeg."/>
  </body>
  </html>


c++ interface
^^^^^^^^^^^^^
Quick start::

  #include <opencv2/core/core.hpp>
  #include "mjpeg_server.hpp"
  void doit()
  {
      using namespace http::server;
      // Run server in background thread.
      std::size_t num_threads = 8;
      std::string doc_root = "./";
      //this initializes the redirect behavor, and the /_all handlers
      server_ptr s = init_streaming_server("0.0.0.0", "9090", doc_root, num_threads);
      streamer_ptr stmr(new streamer);//a stream per image, you can register any number of these.
      register_streamer(s, stmr2, "/stream_0");
      s->start();
      while (true)
      {
        cv::Mat image;
        //fill image somehow here. from camera or something.
        bool wait = false; //don't wait for there to be more than one webpage looking at us.
        int quality = 75; //quality of jpeg compression [0,100]
        int n_viewers = stmr->post_image(image,quality, wait);
        //use boost sleep so that our loop doesn't go out of control.
        boost::this_thread::sleep(boost::posix_time::milliseconds(33)); //30 FPS
      }
  }