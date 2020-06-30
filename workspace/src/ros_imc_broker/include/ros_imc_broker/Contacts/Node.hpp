//***************************************************************************
// Copyright 2007-2017 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Faculdade de Engenharia da             *
// Universidade do Porto. For licensing terms, conditions, and further      *
// information contact lsts@fe.up.pt.                                       *
//                                                                          *
// Modified European Union Public Licence - EUPL v.1.1 Usage                *
// Alternatively, this file may be used under the terms of the Modified     *
// EUPL, Version 1.1 only (the "Licence"), appearing in the file LICENCE.md *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// https://github.com/LSTS/dune/blob/master/LICENCE.md and                  *
// http://ec.europa.eu/idabc/eupl.html.                                     *
//***************************************************************************
// Author: Ricardo Martins                                                  *
//***************************************************************************

#ifndef ROS_IMC_BROKER_CONTACTS_NODE_HPP_INCLUDED_
#define ROS_IMC_BROKER_CONTACTS_NODE_HPP_INCLUDED_

// ISO C++ 98 headers.
#include <string>
#include <vector>
#include <cstdio>
#include <map>

#include <ros_imc_broker/StringUtil.hpp>

namespace ros_imc_broker
{
  namespace contacts
  {
    class Node
    {
    public:
      Node(const std::string& name, const std::string& services):
        name_(name),
        active_(addrs_.end())
      {
        // Search for IMC + UDP services.
        std::vector<std::string> list;
        StringUtil::split(services, ";", list);

        for (unsigned i = 0; i < list.size(); ++i)
        {
          if (list[i].compare(0, 10, "imc+udp://", 10) != 0)
            continue;

          unsigned port = 0;
          char address[128] = {0};

          if (std::sscanf(list[i].c_str(), "%*[^:]://%127[^:]:%u", address, &port) == 2)
            addrs_.insert(std::pair<Address, unsigned>(address, port));
        }
      }

      Node(const Node& node)
      {
        name_ = node.name_;
        addrs_ = node.addrs_;

        if (node.active_ == node.addrs_.end())
          active_ = addrs_.end();
        else
          active_ = addrs_.find(node.active_->first);
      }

      //! Get node name.
      //! @return node name.
      const std::string&
      getName(void) const
      {
        return name_;
      }

      //! Check if address and port are on this node's
      //! list of services.
      //! @param[in] addr address.
      //! @param[in] port port.
      //! @return true if address:port is part of node list
      //! of services, false otherwise.
      bool
      check(const Address& addr, unsigned port)
      {
        std::map<Address, unsigned>::iterator itr;
        itr = addrs_.find(addr);

        return (itr != addrs_.end() && itr->second == port);
      }

      //! Point active address to existing node service.
      //! @param[in] addr node address.
      //! @return true if activation successful, false if
      //! already activated.
      bool
      activate(const Address& addr)
      {
        if (active_ != addrs_.end())
        {
          if (active_->first == addr)
            return false;
        }

        active_ = addrs_.find(addr);
        return (active_ != addrs_.end());
      }

      //! Deactivate destination address from list of services.
      //! @param[in] addr node address.
      //! @return true if deactivation successful, false if already
      //! deactivated.
      bool
      deactivate(const Address& addr)
      {
        if (active_ == addrs_.end())
          return false;

        if (active_->first != addr)
          return false;

        active_ = addrs_.end();
        return true;
      }

      //! Send data to node.
      //! @param[in] sock UDP destination socket.
      //! @param[in] data data to be transmitted.
      //! @param[in] data_len length of data to be transmitted.
      void
      send(UDPSocket& sock, const uint8_t* data, unsigned data_len)
      {
        if (active_ == addrs_.end())
          return;

        try
        {
          sock.write(data, data_len, active_->first, active_->second);
        }
        catch (...)
        { }
      }

    private:
      // Node name.
      std::string name_;
      // Addresses
      std::map<Address, unsigned> addrs_;
      // Active address.
      std::map<Address, unsigned>::iterator active_;
    };
  }
}

#endif
