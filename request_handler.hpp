//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_HANDLER_HPP
#define HTTP_SERVER3_REQUEST_HANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
namespace http
{
  namespace server
  {

    struct reply;
    struct request;
    class connection;

    typedef boost::function<void
    (boost::shared_ptr<connection>, const request&, const std::string&, const std::string&, reply&)> RequestHandlerFnc;

    typedef std::map<std::string, RequestHandlerFnc> RequestHandlerMap;

    /// The common handler for all incoming requests.
    class request_handler: private boost::noncopyable
    {
    public:
      /// Construct with a directory containing files to be served.
      explicit
      request_handler(const std::string& doc_root);

      /// Handle a request and produce a reply.
      void
      handle_request(boost::shared_ptr<connection> conn, const request& req, reply& rep);

      /// Register a request handler for a specific path, and request_type
      /// request_type GET, POST, ....
      void
      register_request_handler(const std::string& path, const std::string& request_type,
                               const RequestHandlerFnc& handler);

      ///get a list of the request handlers registered for the particular request type.
      std::vector<std::string>
      get_request_handlers(const std::string& request_type, const std::string& match) const;
    private:
      /// The directory containing the files to be served.
      std::string doc_root_;
      RequestHandlerMap handlers_;
    public:
      //statics
      /// Perform URL-decoding on a string. Returns false if the encoding was
      /// invalid.
      static bool
      url_decode(const std::string& in, std::string& path, std::string& query);
      static bool
      split_query(const std::string& query, std::vector<std::string>& sv);
      static bool
      split_path_query(const std::string& in, std::string& path, std::string& query);
    };

  } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP
