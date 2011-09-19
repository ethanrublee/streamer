#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mjpeg_server.hpp"

cv::Mat
random_image()
{
  return cv::Mat(cv::Size(640, 480), CV_8UC3, cv::Scalar(std::rand() % 255, std::rand() % 255, std::rand() % 255));
}
int
main(int argc, char* argv[])
{
  try
  {
    // Check command line arguments.
    if (argc != 5)
    {
      std::cerr << "Usage: " << argv[0] << " <address> <port> <threads> <doc_root>\n";
      std::cerr << "try:" << std::endl;
      std::cerr << "   " << argv[0] << " 0.0.0.0 9090 8 .\n";
      return 1;
    }
    using namespace http::server;
    // Run server in background thread.
    std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
    server_ptr s(init_streaming_server(argv[1], argv[2], argv[4], num_threads));
    streamer_ptr stmr(new streamer);
    register_streamer(s, stmr, "/stream_0");
    streamer_ptr stmr2(new streamer);
    register_streamer(s, stmr2, "/stream_1");
    streamer_ptr stmr3(new streamer);
    register_streamer(s, stmr3, "/foobar");
    s->start();
    std::cout << "Visit:\n" << argv[1] << ":" << argv[2] << "/stream_0" << std::endl;
    while (true)
    {
      cv::Mat image1 = random_image(), image2 = random_image(), image3 = random_image();
      bool wait = false; //don't wait for there to be more than one webpage looking at us.
      int quality = 80;
      stmr->post_image(image1,quality, wait);
      stmr2->post_image(image2,quality, wait);
      stmr3->post_image(image3,quality, wait);
      boost::this_thread::sleep(boost::posix_time::milliseconds(100)); //10 fps for ease on my eyes
    }
  } catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }
  return 0;
}
