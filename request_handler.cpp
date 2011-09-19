//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <algorithm>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "connection.hpp"
namespace http
{
  namespace server
  {
    request_handler::request_handler(const std::string& doc_root)
        :
          doc_root_(doc_root)
    {
    }

    void
    request_handler::handle_request(boost::shared_ptr<connection> conn, const request& req, reply& rep)
    {
      // Decode url to path.
      std::string request_path, query_string;
      if (!url_decode(req.uri, request_path, query_string))
      {
        rep = reply::stock_reply(reply::bad_request);
        conn->async_write(rep.to_buffers());
        return;
      }

      // Request path must be absolute and not contain "..".
      if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos)
      {
        rep = reply::stock_reply(reply::bad_request);
        conn->async_write(rep.to_buffers());
        return;
      }

      //first do direct match.
      RequestHandlerMap::iterator h = handlers_.find(request_path + ";" + req.method);
      if (h != handlers_.end())
      {
        h->second(conn, req, request_path, query_string, rep);
        return;
      }
      BOOST_FOREACH(RequestHandlerMap::value_type& regex_handler, handlers_)
          {
            boost::regex e(regex_handler.first);
            if (boost::regex_match(request_path + ";" + req.method, e, boost::match_all))
            {
              regex_handler.second(conn, req, request_path, query_string, rep);
              return;
            }
          }

      // If path ends in slash (i.e. is a directory) then add "index.html".
      if (request_path[request_path.size() - 1] == '/')
      {
        request_path += "index.html";
      }

      // Determine the file extension.
      std::size_t last_slash_pos = request_path.find_last_of("/");
      std::size_t last_dot_pos = request_path.find_last_of(".");
      std::string extension;
      if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
      {
        extension = request_path.substr(last_dot_pos + 1);
      }

      // Open the file to send back.
      std::string full_path = doc_root_ + request_path;
      std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
      if (!is)
      {
        rep = reply::stock_reply(reply::not_found);
        conn->async_write(rep.to_buffers());
        return;
      }
      // Fill out the reply to be sent to the client.
      rep.status = reply::ok;
      char buf[512];
      while (is.read(buf, sizeof(buf)).gcount() > 0)
        rep.content.append(buf, is.gcount());
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = mime_types::extension_to_type(extension);
      conn->async_write(rep.to_buffers());
    }

    bool
    request_handler::split_path_query(const std::string& in, std::string& path, std::string& query)
    {
      using namespace std;
      using namespace boost;
      typedef vector<string> split_vector_type;

      split_vector_type SplitVec; // #2: Search for tokens
      split(SplitVec, in, is_any_of("?")); // SplitVec == { "hello abc","ABC","aBc goodbye" }
      path = SplitVec[0];
      if (SplitVec.size() == 2)
      {
        query = SplitVec[1];
      }
      else
      {
        query = "";
      }
      return true;
    }

    bool
    request_handler::split_query(const std::string& query, std::vector<std::string>& sv)
    {
      using namespace std;
      using namespace boost;
      split(sv, query, is_any_of("&;"));
      return true;
    }
    bool
    request_handler::url_decode(const std::string& in, std::string& out, std::string& query)
    {
      out.clear();
      out.reserve(in.size());
      query.clear();
      for (std::size_t i = 0; i < in.size(); ++i)
      {
        if (in[i] == '%')
        {
          if (i + 3 <= in.size())
          {
            int value = 0;
            std::istringstream is(in.substr(i + 1, 2));
            if (is >> std::hex >> value)
            {
              out += static_cast<char>(value);
              i += 2;
            }
            else
            {
              return false;
            }
          }
          else
          {
            return false;
          }
        }
        else if (in[i] == '+')
        {
          out += ' ';
        }
        else
        {
          out += in[i];
        }
      }
      split_path_query(out, out, query);
      return true;
    }

    void
    request_handler::register_request_handler(const std::string& path, const std::string& request_type,
                                              const RequestHandlerFnc& handler)
    {
      handlers_[path + ";" + request_type] = handler;
    }

    ///get a list of the request handlers registered for the particular request type.
    std::vector<std::string>
    request_handler::get_request_handlers(const std::string& request_type, const std::string& match) const
    {
      std::vector<std::string> handlers;
      boost::regex e(match);
      BOOST_FOREACH(const RequestHandlerMap::value_type& path_handler, handlers_)
          {
            std::string path = path_handler.first;
            std::vector<std::string> sv;
            boost::split(sv, path, boost::is_any_of(";"));
            std::string rt = sv.back();
            sv.pop_back();
            if (rt == request_type)
            {
              std::string path = boost::join(sv, ";");
              if (boost::regex_match(path, e, boost::match_all))
                handlers.push_back(path);
            }
          }
      return handlers;
    }

  } // namespace server3
} // namespace http
