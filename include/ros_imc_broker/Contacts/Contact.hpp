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
// Author: Ricardo Martins (oriinal)                                      *
// Author: Paulo Dias (changed to ROS)                                    *
//*************************************************************************

#ifndef ROS_IMC_BROKER_CONTACTS_CONTACT_HPP_INCLUDED_
#define ROS_IMC_BROKER_CONTACTS_CONTACT_HPP_INCLUDED_

// ISO C++ headers
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>


#include <ros_imc_broker/Network/NetworkUtil.hpp>
#include <ros_imc_broker/Time/Counter.hpp>

namespace ros_imc_broker
{
  namespace Contacts
  {
    //! .
    class Contact
    {
    public:
      Contact(unsigned id, const Network::AddressPort& addr):
        id_(id),
        addr_(addr)
      { }

      Contact(unsigned id, const std::string& addr, const unsigned& port):
        id_(id),
        addr_(Network::AddressPort::create(addr, port))
      { }

      void
      setTimeout(ros::Duration& tout)
      {
        counter_.setTop(tout);
      }

      void
      update(void)
      {
        counter_.reset();
      }

      unsigned
      getId(void) const
      {
        return id_;
      }

      const Network::AddressPort&
      getAddress(void) const
      {
        return addr_;
      }

      bool
      isInactive(void)
      {
        return counter_.overflow();
      }

      bool
      isActive(void)
      {
        return !isInactive();
      }

    private:
      // Node id.
      unsigned id_;
      // Node address.
      Network::AddressPort addr_;
      // Counter to check if node is no longer reachable.
      Time::Counter counter_;
    };
  }
}

#endif