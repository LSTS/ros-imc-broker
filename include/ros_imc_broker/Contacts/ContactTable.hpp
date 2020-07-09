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

#ifndef ROS_IMC_BROKER_CONTACTS_CONTACT_TABLE_HPP_INCLUDED_
#define ROS_IMC_BROKER_CONTACTS_CONTACT_TABLE_HPP_INCLUDED_

// ISO C++ headers
#include <map>
#include <vector>

// ROS headers.
#include <ros/ros.h>

#include <ros_imc_broker/Network/NetworkUtil.hpp>
#include <ros_imc_broker/Time/Counter.hpp>

// Local headers.
#include "Contact.hpp"

namespace ros_imc_broker
{
  namespace Contacts
  {
    class ContactTable
    {
    public:
      ContactTable():
        tout_(30)
      {}

      ContactTable(ros::Duration tout):
        tout_(tout.toSec())
      { }

      ~ContactTable()
      { }

      void
      getContacts(std::vector<Contacts::Contact>& list)
      {
        list.clear();
        for (Table::iterator itr = table_.begin(); itr != table_.end(); ++itr)
          list.push_back(itr->second);
      }

      void
      update(unsigned id, const Network::Endpoint& endpoint)
      {
        update(id, Network::AddressPort::create(endpoint.address().to_string(), endpoint.port()));
      }

      void
      update(unsigned id, const Network::AddressPort& addr)
      {
        Table::iterator itr = table_.find(id);

        if (itr == table_.end())
        {
          std::pair<Table::iterator, bool> rv = table_.insert(Entry(id, Contact(id, addr)));
          itr = rv.first;
          itr->second.setTimeout(tout_);
        }

        // Address has changed... update it.
        if (itr->second.getAddress() != addr)
        {
          itr->second = Contact(id, addr);
          itr->second.setTimeout(tout_);
        }

        itr->second.update();
      }

      void
      setTimeout(ros::Duration tout)
      {
        tout_ = tout;
        for (Table::iterator itr = table_.begin(); itr != table_.end(); ++itr)
          itr->second.setTimeout(tout_);
      }

    private:
      //! Table type.
      typedef std::map<unsigned, Contacts::Contact> Table;
      //! Map entry.
      typedef std::pair<unsigned, Contacts::Contact> Entry;
      //! Table.
      Table table_;
      //! Timeout value to deactivate a contact.
      ros::Duration tout_;
    };
  }
}

#endif