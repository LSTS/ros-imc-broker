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

#ifndef ROS_IMC_BROKER_NETWORK_TCP_CONNECTION_HPP_INCLUDED_
#define ROS_IMC_BROKER_NETWORK_TCP_CONNECTION_HPP_INCLUDED_

// ISO C++ headers
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

// Boost headers.
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

// IMC headers.
#include <IMC/Base/Parser.hpp>

#define MAX_BUFFER_LENGTH (65535)

using boost::asio::ip::tcp;

namespace ros_imc_broker
{
  namespace Network
  {
    class TcpConnection: public boost::enable_shared_from_this<TcpConnection>
    {
    public:
      typedef boost::shared_ptr<TcpConnection> pointer;

      static pointer
      create(boost::asio::io_service& io_service)
      {
        return pointer(new TcpConnection(io_service));
      }

      TcpConnection(boost::asio::io_service& io_service) :
        socket_(io_service),
        handler_(NULL),
        pos_(0),
        msg_size_(-1)
      {}

      ~TcpConnection()
      {}

      void handle_read(boost::system::error_code ec, size_t bytes_read)
      {
        if (ec)
        {
          std::cerr << bytes_read << " connection error: " << ec << std::endl;
          socket_.close();
          return;
        }

        pos_ += bytes_read;

        try
        {
          if (msg_size_ == -1 && pos_ >= IMC_CONST_HEADER_SIZE)
          {
            IMC::Header h;
            IMC::Packet::deserializeHeader(h, &in_data_[0], pos_);
            msg_size_ = IMC_CONST_HEADER_SIZE + h.size + IMC_CONST_FOOTER_SIZE;
          }

          if (msg_size_ > -1 && pos_ >= (unsigned int) msg_size_)
          {
            const IMC::Message* msg = IMC::Packet::deserialize(&in_data_[0], pos_);
            if (handler_ != NULL)
            {
              handler_(msg);
            }
            else
            {
              std::cerr << "message received (ignored): " << msg->getName() << ". " << handler_ << std::endl;
            }
            socket_.close();
          }
          else
          {
            socket_.async_read_some(
                boost::asio::buffer(&in_data_[pos_], 65535 - pos_),
                boost::bind(&TcpConnection::handle_read, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
          }
        }
        catch (std::exception& ex)
        {
          std::cerr << "connection dropped: " << ex.what() << std::endl;
          socket_.close();
        }
      }

      void start()
      {
        pos_ = 0;
        socket_.async_read_some(boost::asio::buffer(&in_data_[0], 65535),
                                 boost::bind(&TcpConnection::handle_read, shared_from_this(),
                                     boost::asio::placeholders::error,
                                     boost::asio::placeholders::bytes_transferred));
      }

      void setMessageHandler(handler_t handler)
      {
        handler_ = handler;
      }

      tcp::socket&
      getSocket()
      {
        return socket_;
      }

    private:
      tcp::socket socket_;
        //! Max buffer size enum
      enum { max_length = MAX_BUFFER_LENGTH };
      uint8_t in_data_[max_length];
      handler_t handler_;
      uint pos_;
      int msg_size_;
    };
  }
}

#endif
