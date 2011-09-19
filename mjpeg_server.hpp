#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <opencv2/core/core.hpp>

#include "server.hpp"

namespace http
{
  namespace server
  {
    typedef boost::shared_ptr<server> server_ptr;
    struct streamer: boost::enable_shared_from_this<streamer>
    {
      streamer();
      void
      handle_initial_header_write(connection_ptr conn, const boost::system::error_code& e);
      void
      handle_stream(connection_ptr conn, const request& req, const std::string& path, const std::string& query,
                    reply& rep);
      /**
       * Post an image in a thread safe manner to the streamer. This will be broadcast
       * to any page that is viewing the URL that this was registered to the server with.
       * @param image the image to encode into a jpeg.
       * @param quality the jpeg compression quality, [0,100]
       * @param wait block unless at least someone is viewing the url associated with this streamer.
       * @return The number of viewers that theoretically are viewing this stream
       */
      int
      post_image(const cv::Mat& image, int quality, bool wait = false);
      boost::condition_variable cond_;
      boost::mutex mtx_;
      std::vector<uint8_t> jpg_buffer_;
      int watchers_;
    };

    typedef boost::shared_ptr<streamer> streamer_ptr;

    std::vector<std::string>
    list_all_streams(server_ptr serv, const std::string& prefix, const std::string&path);

    void
    register_streamer(server_ptr serv, streamer_ptr stmr, const std::string& path);

    server_ptr
    init_streaming_server(const std::string& address, const std::string& port, const std::string& doc_root,
                          std::size_t thread_pool_size);
  }
}
