//*************************************************************************
// Copyright (C) 2017-2018 FEUP-LSTS - www.lsts.pt                        *
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

#ifndef ROS_IMC_BROKER_UDP_LINK_HPP_INCLUDED_
#define ROS_IMC_BROKER_UDP_LINK_HPP_INCLUDED_

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

#define MULTICAST_IP "224.0.75.69"
#define MULTICAST_PORT (30100)
#define MULTICAST_PORT_MAX_INDEXES (5)
#define WORKING_THREADS (3)
#define MAX_BUFFER_LENGTH (65535)

namespace ros_imc_broker
{
  namespace Network
  {
    typedef boost::asio::ip::udp::endpoint Endpoint;

    //! UDP link to Neptus.
    //
    //! This class implements a UDP server that can be used as normal
    //! unicast or mullticast.
    //!
    //! @author Paulo Dias <pdias@lsts.pt>
    class UdpLink
    {
    public:
      //! Constructor for default multicast port. It starts automaticaly.
      //! @param[in] recv_handler handler function for received messages.
      UdpLink(boost::function<void (IMC::Message*, Endpoint*)> recv_handler):
        socket_(io_service_, boost::asio::ip::udp::v4()),
        recv_handler_(recv_handler),
        connected_(false),
        multicast_(true),
        multicast_on_(false),
        broadcast_(true),
        multicast_addr_(MULTICAST_IP),
        port_(MULTICAST_PORT),
        port_range_(MULTICAST_PORT_MAX_INDEXES)
      {
        start();
      }

      //! Constructor for multicast port. It starts automaticaly.
      //! @param[in] recv_handler handler function for received messages.
      //! @param[in] multicast_addr the multicast group to bind to.
      //! @param[in] port the port to bind to.
      //! @param[in] port_range the port range to use, it will bind to one of it and send to all.
      //! @param[in] broadcast if broadcast is used or not.
      UdpLink(boost::function<void (IMC::Message*, Endpoint*)> recv_handler,
          std::string multicast_addr, int port, int port_range = 1, bool broadcast = true):
        socket_(io_service_, boost::asio::ip::udp::v4()),
        recv_handler_(recv_handler),
        connected_(false),
        multicast_(true),
        multicast_on_(false),
        broadcast_(broadcast),
        multicast_addr_(multicast_addr),
        port_(port),
        port_range_(port_range)
      {
        start();
      }

      //! Constructor for non multicast port. It starts automaticaly.
      //! @param[in] recv_handler handler function for received messages.
      //! @param[in] port the port to bind to.
      //! @param[in] port_range the port range to use.
      //! @param[in] broadcast if broadcast is used or not.
      UdpLink(boost::function<void (IMC::Message*, Endpoint*)> recv_handler,
          int port, int port_range = 1, bool broadcast = false):
        socket_(io_service_, boost::asio::ip::udp::v4()),
        recv_handler_(recv_handler),
        connected_(false),
        multicast_(false),
        multicast_on_(false),
        broadcast_(broadcast),
        multicast_addr_(""),
        port_(port),
        port_range_(port_range)
      {
        start();
      }

      ~UdpLink(void)
      {
        stop();
      }

      bool
      start()
      {
        connect();
        if (isConnected())
          receive();

        for (int i = 0; i < WORKING_THREADS; i++)
          worker_threads_.create_thread(boost::bind(&ros_imc_broker::Network::UdpLink::ioRunner, this));

        ROS_INFO("UDP started");
      }

      bool
      stop()
      {
        if (isConnected())
        {
          boost::system::error_code ec;
          socket_.shutdown(boost::asio::ip::udp::socket::shutdown_both, ec);
          socket_.close(ec);
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

      bool
      isMulticast(void)
      {
        return multicast_;
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
            Endpoint endpoint(boost::asio::ip::address::from_string(destination_addr), pt);
            socket_.async_send_to(
              boost::asio::buffer(out_buffer, rv),
              endpoint,
              boost::bind(&ros_imc_broker::Network::UdpLink::handle_send_to, this,
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
      boost::function<void (IMC::Message*, Endpoint*)> recv_handler_;
      //! UDP socket.
      boost::asio::ip::udp::socket socket_;
      //! Received message endpoint
      Endpoint endpoint_;
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
      //! Indicates if is to use nulticast
      bool multicast_;
      //! Indicates if nulticast is in use
      bool multicast_on_;
      //! Indicates if socket is connected
      bool connected_;
      //! Indicates if socket is to have broadcast enabled
      bool broadcast_;

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
        multicast_on_ = false;

        ROS_INFO("connecting to %s@%d", multicast_ ? multicast_addr_.c_str() : "", port_);

        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        if (broadcast_)
          socket_.set_option(boost::asio::ip::udp::socket::broadcast(true));
        bool bound = false;
        int port_used = port_;
        for (int i = 0; i < port_range_; i++)
        {
          try
          {
            port_used = port_ + i;
            socket_.bind(Endpoint(boost::asio::ip::udp::v4(), port_used));
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

        if (multicast_)
        {
          try
          {
            // Join the multicast group.
            socket_.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address::from_string(multicast_addr_)));
            multicast_on_ = true;
          }
          catch (std::exception &ex)
          {
            multicast_on_ = false;
            std::cerr << "Error binding to multicast group " << multicast_addr_.c_str() << std::endl;
          }
        }

        connected_ = true;
        ROS_INFO("connected to %s%s%s@%d",
            multicast_on_ ? "multicast " : "",
            broadcast_ ? "broadcast " : "",
            multicast_ ? multicast_addr_.c_str() : "",
            port_used);
      }

      void
      receive(void)
      {
        socket_.async_receive_from(
            boost::asio::buffer(in_buffer_, max_length),
            endpoint_,
            boost::bind(&ros_imc_broker::Network::UdpLink::handle_receive_from, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
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
                if (recv_handler_ != NULL)
                {
                  Endpoint* ep;
                  ep = new Endpoint(endpoint_);
                  recv_handler_(m, ep);
                }
                else
                {
                  ROS_DEBUG("No handler to process message %s received on UDP.", m->getName());
                }
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