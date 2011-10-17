#include <ecto/ecto.hpp>
#include "mjpeg_server.hpp"
#include "server.hpp"
#include "mime_types.hpp"
#include <boost/foreach.hpp>
using ecto::tendrils;
using namespace http::server;

namespace mjpeg_server
{
  void
  wrap_server();
  struct MJPEGServer
  {
    static void
    declare_params(tendrils& p)
    {
      p.declare<server_ptr>("server", "The http server to use.").required(true);
      p.declare<std::string>("path", "The path which will resolve to the image.").required(true);
      p.declare<int>("quality", "The compression level.", 75);
      p.declare<bool>("wait", "Wait for webbrowser clients. If this is true then execution"
                      " will block unless a browser window is viewing the cell.",
                      false);

    }
    static void
    declare_io(const tendrils& p, tendrils& i, tendrils& o)
    {
      i.declare<cv::Mat>("image", "The image to post to the server.").required(true);
    }
    void
    configure(const tendrils& p, const tendrils& i, const tendrils& o)
    {
      streamer_.reset(new streamer);
      p["server"] >> server_;
      p["path"] >> path_;
      image_ = i["image"];
      wait_ = p["wait"];
      quality_ = p["quality"];
      if (!server_)
        throw std::runtime_error("You must supply a server instance.");
      register_streamer(server_, streamer_, path_);
    }
    int
    process(const tendrils& i, const tendrils& o)
    {
      if (!image_->empty())
      {
        streamer_->post_image(*image_, *quality_, *wait_);
      }
      return ecto::OK;
    }
    server_ptr server_;
    streamer_ptr streamer_;
    std::string path_;
    ecto::spore<cv::Mat> image_;
    ecto::spore<bool> wait_;
    ecto::spore<int> quality_;
  };

  template<typename T>
  bool
  repr_t(const ecto::tendril& t, std::string& repr)
  {
    bool isit = t.is_type<T>();
    if (isit)
    {
      repr = boost::lexical_cast<std::string>(t.get<T>());
    }
    return isit;
  }

  std::string
  repr_tendril(const ecto::tendril& t)
  {

    std::string repr = "NA";
    if (repr_t<bool>(t, repr))
    {
    }
    else if (repr_t<int>(t, repr))
    {
    }
    else if (repr_t<unsigned>(t, repr))
    {
    }
    else if (repr_t<std::string>(t, repr))
    {
    }
    else if (repr_t<double>(t, repr))
    {
    }
    else if (repr_t<float>(t, repr))
    {
    }
    return repr;
  }

  void
  handle_get_cell_params(ecto::cell::ptr c, connection_ptr conn, const request& req, const std::string& path,
                         const std::string&query, reply&rep)
  {
    rep.status = reply::ok;
    rep.headers.clear();
    rep.content.clear();
    rep.content += "{";
    typedef std::pair<std::string, ecto::tendril_ptr> value_type;
    BOOST_FOREACH(value_type v, c->parameters)
        {
          rep.content += v.first + ":" + repr_tendril(*v.second) + ",";
        }
    rep.content += "}";
    rep.headers.push_back(header("Content-Length", boost::lexical_cast<std::string>(rep.content.size())));
    rep.headers.push_back(header("Content-Type", mime_types::extension_to_type("txt")));
    conn->async_write(rep.to_buffers());
  }

  void
  register_cell_params(server_ptr serv, boost::python::object obj, const std::string& path)
  {
    namespace bp = boost::python;
    ecto::cell_ptr cell = bp::extract<ecto::cell_ptr>(bp::getattr(obj, "__impl"));
    serv->register_request_handler(path, "GET", boost::bind(handle_get_cell_params, cell, _1, _2, _3, _4, _5));
  }

  void
  wrap_server()
  {
    using namespace boost::python;
    class_<server, server_ptr, boost::noncopyable> serv("server", no_init);
    serv.def("start", &server::start);
    serv.def("stop", &server::stop);
    serv.def("register_cell", register_cell_params);

    def("server",
        init_streaming_server,
        (arg("address") = "0.0.0.0", arg("port") = "9090", arg("doc_root") = "./", arg("nthreads") = 8),
        "Create a simple asio based http server. This may be used to serve any document based at doc_root, or possibly for streaming jpeg images...");
  }

}

ECTO_DEFINE_MODULE(mjpeg_server)
{
  mjpeg_server::wrap_server();
}
ECTO_CELL(mjpeg_server, mjpeg_server::MJPEGServer, "Streamer",
          "A single image streaming cell, that will post to the mjpeg server instance that is running.");

