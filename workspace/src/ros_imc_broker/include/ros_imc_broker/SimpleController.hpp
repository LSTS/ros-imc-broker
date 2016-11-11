//*************************************************************************
// Copyright (C) 2016 OceanScan - Marine Systems & Technology, Lda.       *
//*************************************************************************
// This program is free software; you can redistribute it and/or modify   *
// it under the terms of the GNU General Public License as published by   *
// the Free Software Foundation; either version 2 of the License, or (at  *
// your option) any later version.                                        *
//                                                                        *
// This program is distributed in the hope that it will be useful, but    *
// WITHOUT ANY WARRANTY; without even the implied warranty of             *
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
// General Public License for more details.                               *
//                                                                        *
// You should have received a copy of the GNU General Public License      *
// along with this program; if not, write to the Free Software            *
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          *
// 02110-1301 USA.                                                        *
//*************************************************************************
//Author: Ricardo Martins                                                 *
//*************************************************************************

#ifndef ROS_IMC_BROKER_SIMPLE_CONTROLLER_HPP_INCLUDED_
#define ROS_IMC_BROKER_SIMPLE_CONTROLLER_HPP_INCLUDED_

// ROS headers.
#include <ros/ros.h>

// DUNE headers.
#include <DUNE/DUNE.hpp>

// Local headers.
#include <ros_imc_broker/ImcTypes.hpp>

namespace ros_imc_broker
{
  class SimpleController
  {
  public:
    SimpleController(ros::NodeHandle& nh, const std::string& system_name):
      nh_(nh),
      system_name_(system_name),
      system_id_(-1),
      service_(false),
      state_(SM_BEGIN)
    {
      // Subscribers.
      subscribe<SimpleController, DUNE::IMC::Announce>("IMC/In/Announce", this);
      subscribe<SimpleController, DUNE::IMC::VehicleState>("IMC/In/VehicleState", this);
    }

    virtual
    ~SimpleController(void)
    { }

    void
    updateStateMachine(void)
    {
      switch (state_)
      {
        case SM_BEGIN:
          ROS_INFO("resolving system identifier");
          state_ = SM_RESOLVE;
          break;

        case SM_RESOLVE:
          if (systemIsResolved())
            state_ = SM_SERVICE;
          break;

        case SM_SERVICE:
          if (systemIsInService())
            state_ = SM_EXTERNAL;
          break;

        case SM_EXTERNAL:
          updateExternalStateMachine();
          break;
      }
    }

  protected:
    const char*
    getSystemName(void) const
    {
      return system_name_.c_str();
    }

    const unsigned
    getSystemId(void) const
    {
      return (unsigned)system_id_;
    }

    template<typename M, typename T>
    void
    subscribe(const std::string& topic, M* m)
    {
      ros::Subscriber sub = nh_.subscribe(topic, 1000, static_cast<void (M::*)(const T&)>(&M::on), m);
      m_subs.insert(std::pair<std::string, ros::Subscriber>(topic, sub));
    }

    //! Tests if a given message was sent by the controlled system.
    //! @return true if message is from controlled system, false otherwise.
    bool
    isFromControlledSystem(const DUNE::IMC::Message& msg) const
    {
      return msg.getSource() == system_id_;
    }

    virtual void
    updateExternalStateMachine(void)
    { }

  private:
    enum State
    {
      SM_BEGIN,
      SM_RESOLVE,
      SM_SERVICE,
      SM_EXTERNAL
    };

    //! ROS node handle.
    ros::NodeHandle& nh_;
    //! Canonical name of the controlled system.
    std::string system_name_;
    //! IMC identifier of the controlled system.
    int system_id_;
    //! Subscriber map.
    std::map<std::string, ros::Subscriber> m_subs;
    //! True if system is in service mode.
    bool service_;
    //! Current finite state machine state.
    State state_;

    //! Tests if the IMC identifier of the controlled system has been
    //! resolved.
    //! @return true if IMC identifier is known, false otherwise.
    bool
    systemIsResolved(void) const
    {
      return system_id_ > 0;
    }

    //! Tests if the controlled system is idle ready to accept commands.
    //! @return true if controlled system is ready, false otherwise.
    bool
    systemIsInService(void) const
    {
      return service_;
    }

    //! Handles system announcement messages.
    //! @param[in] msg IMC announcement messages.
    void
    on(const DUNE::IMC::Announce& msg)
    {
      if (!systemIsResolved() && msg.sys_name == system_name_)
      {
        system_id_ = msg.getSource();
        ROS_INFO("resolved system identifier: 0x%04x", (unsigned)system_id_);
      }
    }

    void
    on(const DUNE::IMC::VehicleState& msg)
    {
      if (!isFromControlledSystem(msg))
        return;

      bool service = msg.op_mode == DUNE::IMC::VehicleState::VS_SERVICE;
      if (service_ && !service)
        ROS_INFO("system is not idle");
      else if (!service_ && service)
        ROS_INFO("system is idle");

      service_ = service;
    }
  };
}

#endif
