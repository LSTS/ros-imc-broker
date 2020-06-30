//*************************************************************************
// Copyright (C) 2017 FEUP-LSTS - www.lsts.pt                             *
//*************************************************************************
// This program is free software; you can redistribute it and/or modify   *
// it under the terms of the GNU General Public License as published by   *
// the Free Software Foundation; either version 2 of the License, or (at  *
// your option) any later version.                                        *
//                                                                        *
// This program is distributed in the hope that it will be useful, but    *
// WITHOUT ANY WARRANTY; without even the implied warranty of             *
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
// General Public License for more details.                               *
//                                                                        *
// You should have received a copy of the GNU General Public License      *
// along with this program; if not, write to the Free Software            *
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          *
// 02110-1301 USA.                                                        *
//*************************************************************************
// Author: Paulo Dias                                                     *
//*************************************************************************

#ifndef ROS_IMC_BROKER_NETWORK_TCP_SERVER_LINK_HPP_INCLUDED_
#define ROS_IMC_BROKER_NETWORK_TCP_SERVER_LINK_HPP_INCLUDED_

// ISO C++ headers
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

// Boost headers.
#include <boost/asio.hpp>

// IMC headers.
#include <IMC/Base/Parser.hpp>

// ROS headers.
#include <ros/ros.h>

#include <ros_imc_broker/Network/TcpConnection.hpp>

#define WORKING_THREADS (3)
#define MAX_BUFFER_LENGTH (65535)

namespace ros_imc_broker
{
  namespace Network
  {
    //! TCP Server link to Neptus.
    //
    //! This class implements a TCP server.
    //!
    //! @author Paulo Dias <pdias@lsts.pt>
    class TcpServerLink
    {
    public:
      //! Constructor.
      //! @param[in] recv_handler handler function for received messages.
      //! @param[in] port the port to bind to.
      //! @param[in] port_range the port range to use.
      TcpServerLink(boost::function<void (IMC::Message*)> recv_handler, int port,
          int port_range = 1):
        socket_(io_service_, boost::asio::ip::tcp::v4()),
        recv_handler_(recv_handler),
        connected_(false),
        port_(port),
        port_range_(port_range)
      {
        start();
      }

      ~TcpServerLink(void)
      {
        stop();
      }

      bool
      start()
      {
        connect();
        if (isConnected())
          accept();

        for (int i = 0; i < WORKING_THREADS; i++)
          worker_threads_.create_thread(boost::bind(&ros_imc_broker::TcpServerLink::ioRunner, this));

        ROS_INFO("UDP started");
      }

      bool
      stop()
      {
        if (isConnected())
        {
          boost::system::error_code ec;
          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
          socket_.close(ec);
          acceptor_->cancel();
          io_service_.stop();
          worker_threads_.join_all();

          connected_ = false;
        }
      }

      bool
      isConnected(void)
      {
        return connected_;
      }

      int
      bindedPort(void)
      {
        return port_binded_;
      }

      void
      send(const IMC::Message *message)
      {
        send(message, multicast_addr_, port_, port_range_);
      }

      void
      send(const IMC::Message *message, std::string destination_addr, int port)
      {
        send(message, destination_addr, port, 1);
      }

      void
      send(const IMC::Message *message, std::string destination_addr, int port, int port_range)
      {
        char out_buffer[max_length];
        uint16_t rv = IMC::Packet::serialize(message, (uint8_t*)out_buffer, sizeof(out_buffer));

        for (int pt = port; pt < port + port_range; pt++)
        {
          try
          {
            boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(destination_addr), pt);
            socket_.async_send_to(
              boost::asio::buffer(out_buffer, rv),
              endpoint,
              boost::bind(&ros_imc_broker::TcpServerLink::handle_send_to, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
            
            ROS_INFO("sending %s to %s:%d sent %d", message->getName(), destination_addr.c_str(), pt, (int)rv);
          }
          catch (std::exception& e)
          {
            ROS_WARN("Error %s sending to %s@%d: %s", message->getName(), destination_addr.c_str(), pt, e.what());
          }
        }
      }

    private:
      //! IO service to run the asysnc UDP.
      boost::asio::io_service io_service_;
      //! Thread for IO service use.
      boost::thread_group worker_threads_;
      //! Callback for received IMC messages.
      boost::function<void (IMC::Message*)> recv_handler_;
      
      //! TCP socket.
      boost::asio::ip::tcp::socket socket_;
      //! Received message endpoint
      std::map<std::string, boost::asio::ip::tcp::endpoint network_addr_;
      std::map<int, std::string> network_ids_;
      int send_request_;
      boost::asio::ip::tcp::acceptor * acceptor_;

      //! Max buffer size enum
      enum { max_length = MAX_BUFFER_LENGTH };
      //! Incoming data buffer.
      char in_buffer_[max_length];

      //! IMC message parser.
      IMC::Parser parser_;
      //! Multicast address
      std::string multicast_addr_;
      //! Port to cennect to
      int port_;
      //! Port range to use
      int port_range_;
      //! Port binded to
      int port_binded_;
      //! Indicates if socket is connected
      bool connected_;

      void
      ioRunner(void)
      {
        try
        {
          io_service_.run();
        }
        catch (std::exception & ex)
        {
          std::cerr << "[" << boost::this_thread::get_id() << "] Exception: "
              << ex.what() << std::endl;
        }
      }

      void
      connect(void)
      {
        ROS_INFO("connecting to @%d", port_);

        bool bound = false;
        int port_used = port_;
        for (int i = 0; i < port_range_; i++)
        {
          try
          {
            port_used = port_ + i;
            acceptor_ = new boost::asio::ip::tcp::acceptor(io_service_, boost::asio::ip::tcp::endpoint(tcp::v4(), port));
            acceptor_->listen();

            bound = true;
            port_binded_ = port_used;
            break;
          }
          catch (std::exception &ex)
          {
            std::cerr << "Error binding to port " << port_used << std::endl;
          }
        }

        if (!bound)
            throw std::runtime_error("Could not bind to any port.");

        connected_ = true;
        ROS_INFO("connected to @%d",
            port_used);
      }

      void
      accept()
      {
        if (!isConnected())
        {
          std::cerr << "message dropped because comms are disabled. " << std::endl;
          acceptor_->cancel();
          return;
        }

        TcpConnection::pointer conn = TcpConnection::create(io_service_);
        conn->setMessageHandler(m_handler);
        m_acceptor->async_accept(
            conn->getSocket(),
            boost::bind(&TcpServerLink::handle_accept, this, conn, _1));
      }

      void
      handle_accept(TcpConnection::pointer conn, const boost::system::error_code& ec)
      {
        if (!ec)
        {
          conn->start();
        }
        else
        {
          std::cerr << "error in accept: " << ec.message() << std::endl;
        }

        accept();
      }

      void
      InterNodeComms::handle_connect(const boost::system::error_code& ec,
          boost::shared_ptr<std::vector<uint8_t> > data,
          boost::shared_ptr<tcp::socket> sock, int request_id, send_handler_t handler)
      {
        if (!ec)
        {
          sock->async_send(
              boost::asio::buffer(*data),
              boost::bind(&ros_imc_broker::handle_send, this, _1, _2, request_id,
                  handler));
        }
        else {
          handler(request_id, false, "Error connecting with remote peer.");
        }
      }

      void
      InterNodeComms::handle_send(const boost::system::error_code& ec,
          size_t bytes_transferred, int request_id, send_handler_t handler)
      {
        (void) bytes_transferred;
        if (ec)
          handler(request_id, false, "Error while sending data to remote peer.");
        else
          handler(request_id, true, "");
      }

      void
      handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
      {
        try
        {
          if (!error && bytes_recvd > 0)
          {
            size_t rv = bytes_recvd;
            for (size_t i = 0; i < rv; ++i)
            {
              IMC::Message* m = parser_.parse((uint8_t)in_buffer_[i]);
              if (m)
              {
                ROS_INFO("received %s message from %s@%d srcID 0x%04x", 
                    m->getName(), endpoint_.address().to_string().c_str(), 
                    endpoint_.port(), (unsigned)m->getSource());

                recv_handler_(m);
                delete m;
              }
              else
              {
                ROS_DEBUG("Invalid message received on UDP. Bytes received %d", (int)bytes_recvd);
              }
            }

          }
        }
        catch (std::exception &ex)
        {
          std::cerr << "[" << boost::this_thread::get_id() << "] Exception: "
              << ex.what() << std::endl;
        }

        receive();
      }

      void
      handle_send_to(const boost::system::error_code& error, size_t bytes_sent)
      {
        try
        {
          if (!error && bytes_sent > 0)
          {
            ROS_DEBUG("sent %d", (int)bytes_sent);
          }
          else
          {
            ROS_ERROR("Error sending message %d :: %s", error.value(), error.message().c_str());
          }
        }
        catch (std::exception &ex)
        {
          std::cerr << "[" << boost::this_thread::get_id() << "] Exception: "
              << ex.what() << std::endl;
        }
      }
    };
  }
}

#endif