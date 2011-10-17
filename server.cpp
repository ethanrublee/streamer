//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <iostream>
#include "connection.hpp"
#include <boost/lexical_cast.hpp>
#include "mime_types.hpp"
namespace http
{
  namespace server
  {
    typedef boost::shared_ptr<server> server_ptr;
    typedef boost::shared_ptr<connection> connection_ptr;

    void handle_jquery(server_ptr serv, connection_ptr conn, const request& req, const std::string& path,
                    const std::string&query, reply&rep)
    {
#include "jquery/jquery.i"
      static const std::string jqst(JQUERY_STR, sizeof(JQUERY_STR));
      //this redirect the browser so it thinks the stream url is unique
      rep.status = reply::ok;
      rep.headers.clear();
      rep.content = jqst;
      rep.headers.push_back(header("Content-Length", boost::lexical_cast<std::string>(sizeof(JQUERY_STR))));
      rep.headers.push_back(header("Content-Type", mime_types::extension_to_type("js")));
      conn->async_write(rep.to_buffers());
    }
    void
    register_json(server_ptr serv)
    {
      serv->register_request_handler("/jquery.js", "GET",
                                     boost::bind(handle_jquery, serv, _1, _2, _3, _4, _5));
    }

    server::server(const std::string& address, const std::string& port, const std::string& doc_root,
                   std::size_t thread_pool_size)
        :
          thread_pool_size_(thread_pool_size),
          acceptor_(io_service_),
          new_connection_(new connection(io_service_, request_handler_)),
          request_handler_(doc_root)
    {
      // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
      boost::asio::ip::tcp::resolver resolver(io_service_);
      boost::asio::ip::tcp::resolver::query query(address, port);
      boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
      acceptor_.open(endpoint.protocol());
      acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
      acceptor_.bind(endpoint);
      acceptor_.listen();
      acceptor_.async_accept(new_connection_->socket(),
                             boost::bind(&server::handle_accept, this, boost::asio::placeholders::error));
    }

    void
    server::run()
    {
      // Create a pool of threads to run all of the io_services.
      std::vector<boost::shared_ptr<boost::thread> > threads;
      for (std::size_t i = 0; i < thread_pool_size_; ++i)
      {
        boost::shared_ptr<boost::thread> thread(
            new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_)));
        threads.push_back(thread);
      }

      // Wait for all threads in the pool to exit.
      for (std::size_t i = 0; i < threads.size(); ++i)
        threads[i]->join();
    }

    void
    server::start()
    {
      register_json(shared_from_this());
      run_thread_.reset(new boost::thread(boost::bind(&server::run, shared_from_this())));
    }

    void
    server::stop()
    {
      io_service_.stop();
      if (run_thread_)
      {
        run_thread_->interrupt();
        run_thread_->join();
        run_thread_.reset();
      }
    }

    void
    server::handle_accept(const boost::system::error_code& e)
    {
      if (!e)
      {
        connection_ptr nc(new connection(io_service_, request_handler_));
        nc.swap(new_connection_);
        acceptor_.async_accept(new_connection_->socket(),
                               boost::bind(&server::handle_accept, this, boost::asio::placeholders::error));
        nc->start();

      }
    }

    void
    server::register_request_handler(const std::string& path, const std::string& request_type,
                                     const RequestHandlerFnc& handler)
    {
      request_handler_.register_request_handler(path, request_type, handler);
    }

    std::vector<std::string>
    server::get_request_handlers(const std::string& request_type, const std::string& match) const
    {
      return request_handler_.get_request_handlers(request_type, match);
    }

  } // namespace server3
} // namespace http
