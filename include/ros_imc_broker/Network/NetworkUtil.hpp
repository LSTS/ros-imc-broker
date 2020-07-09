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

#ifndef ROS_IMC_BROKER_NETWORK_UTIL_HPP_INCLUDED_
#define ROS_IMC_BROKER_NETWORK_UTIL_HPP_INCLUDED_

// ISO C++ headers
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

// Boost headers.
#include <boost/asio.hpp>

namespace ros_imc_broker
{
  namespace Network
  {
    struct AddressPort
    {
      // Address.
      std::string addr_;
      // Port.
      unsigned port_;

      static
      AddressPort
      create(std::string& addr, unsigned& port)
      {
        AddressPort ap;
        ap.addr_ = addr;
        ap.port_ = port;
        return ap;
      }

      static
      AddressPort
      create(const std::string& addr, const unsigned& port)
      {
        AddressPort ap;
        ap.addr_.append(addr);
        ap.port_ = port;
        return ap;
      }

      bool
      operator==(const AddressPort& other) const
      {
        return (addr_.compare(other.addr_) == 0 && port_ == other.port_);
      }

      bool
      operator!=(const AddressPort& other) const
      {
        return (addr_.compare(other.addr_) != 0 || port_ != other.port_);
      }
    };

    //!
    //! @author Paulo Dias <pdias@lsts.pt>
    class NetworkUtil
    {
    public:

      static
      std::vector<boost::asio::ip::address>
      getNetworkInterfaces()
      {
        std::vector<boost::asio::ip::address> address_list;

        boost::asio::io_service io_service;

        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::host_name(), "");
        boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

        while(it != boost::asio::ip::tcp::resolver::iterator())
        {
            try
            {
              boost::asio::ip::address addr = (it++)->endpoint().address();
              if(addr.is_v4())
              {
                std::cout << "IPv4 address: " << addr.to_string() << std::endl;
                address_list.push_back(addr);
              }
            }
            catch (std::exception & ex)
            {
              std::cerr << "[" << boost::this_thread::get_id() << "] Exception: "
                  << ex.what() << std::endl;
            }
        }
        std::cout << "Found " << address_list.size() << " IPv4 address" << std::endl;
        return address_list;
      }

    private:

    };
  }
}

#endif