//
// based on:
// posix_main.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// modified by Ethan Rublee, erublee@willowgarage.com September, 2011
// adding mjpeg streaming
//

#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>

#include <opencv2/core/core.hpp>

#include "mjpeg_server.hpp"

using namespace http::server3;

int
main(int argc, char* argv[])
{
  try
  {
    // Check command line arguments.
    if (argc != 5)
    {
      std::cerr << "Usage: http_server <address> <port> <threads> <doc_root>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    receiver 0.0.0.0 80 1 .\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    receiver 0::0 80 1 .\n";
      return 1;
    }
    // Run server in background thread.
    std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
    server_ptr s(init_streaming_server(argv[1], argv[2], argv[4], num_threads));
    streamer_ptr stmr(new streamer);
    register_streamer(s, stmr, "/stream_0");
    s->start();
    std::cout << "Visit:\n" << argv[1] << ":" << argv[2] << "/stream_0" << std::endl;
    while (true)
    {
      cv::Mat image(cv::Size(640, 480), CV_8UC3, cv::Scalar(std::rand() % 255, std::rand() % 255, std::rand() % 255));
      bool wait = true; //wait for there to be more than one webpage looking at us.
      stmr->post_image(image, wait);
      //sleep half a second.
      boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    }
  } catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
