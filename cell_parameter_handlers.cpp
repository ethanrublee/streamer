#include <ecto/ecto.hpp>
#include "server.hpp"
#include "mime_types.hpp"
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include "json_spirit/json_spirit.h"
using ecto::tendrils;
using namespace http::server;

namespace mjpeg_server
{
  typedef boost::shared_ptr<server> server_ptr;

  template<typename T>
  bool
  repr_t(const ecto::tendril& t, json_spirit::Value& repr)
  {
    bool isit = t.is_type<T>();
    if (isit)
    {
      repr = json_spirit::Value(t.get<T>());
    }
    return isit;
  }

  template<typename T>
  bool
  set_t(ecto::tendril& t, const json_spirit::Value& val)
  {
    bool isit = t.is_type<T>();
    if (isit)
    {
      t << val.get_value<T>();
    }
    return isit;
  }
  json_spirit::Value
  repr_tendril(const ecto::tendril& t)
  {
    json_spirit::Value repr;
    if (repr_t<bool>(t, repr))
    {
    }
    else if (repr_t<int>(t, repr))
    {
    }
    else if (repr_t<size_t>(t, repr))
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
  set_tendril(ecto::tendril& t,  json_spirit::Value& x)
  {
    if (set_t<bool>(t, x))
    {
    }
    else if (set_t<int>(t, x))
    {
    }
    else if (set_t<size_t>(t, x))
    {
    }
    else if (set_t<std::string>(t, x))
    {
    }
    else if (set_t<double>(t, x))
    {
    }
//    else if (set_t<float>(t, x))
//    {
//    }
  }

  void
  handle_get_cell_params(ecto::cell::ptr c, connection_ptr conn, const request& req, const std::string& path,
                         const std::string&query, reply&rep)
  {
    rep.status = reply::ok;
    rep.headers.clear();
    rep.content.clear();
    json_spirit::Object obj;
    typedef std::pair<std::string, ecto::tendril_ptr> value_type;
    obj.push_back(json_spirit::Pair("__name__",c->name()));
    obj.push_back(json_spirit::Pair("__doc__",c->short_doc()));
    BOOST_FOREACH(value_type v, c->parameters)
        {
          obj.push_back(json_spirit::Pair(v.first,repr_tendril(*v.second)));
          obj.push_back(json_spirit::Pair(v.first +"_doc",v.second->doc()));
        }
    rep.content += json_spirit::write_formatted(obj);
    rep.headers.push_back(header("Content-Length", boost::lexical_cast<std::string>(rep.content.size())));
    rep.headers.push_back(header("Content-Type", mime_types::extension_to_type("txt")));
    conn->async_write(rep.to_buffers());
  }

  void
  handle_post_cell_params(ecto::cell::ptr c, connection_ptr conn, const request& req, const std::string& path,
                          const std::string&query, reply&rep)
  {
    rep.status = reply::ok;
    rep.headers.clear();
    rep.content.clear();
    std::vector<std::string> query_vec;
    boost::split(query_vec,query,boost::is_any_of("=&;"));

    json_spirit::Value val;
    json_spirit::read(query_vec[1],val);
    std::cout << json_spirit::write_formatted(val) << std::endl;
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
    serv->register_request_handler(path, "POST", boost::bind(handle_post_cell_params, cell, _1, _2, _3, _4, _5));
  }

}
