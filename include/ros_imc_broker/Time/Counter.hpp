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

#ifndef ROS_IMC_BROKER_TIME_COUNTER_HPP_INCLUDED_
#define ROS_IMC_BROKER_TIME_COUNTER_HPP_INCLUDED_

// ISO C++ headers
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

// ROS headers.
#include <ros/ros.h>

namespace ros_imc_broker
{
  namespace Time
  {
    //! Simple time counter. This class checks if the amount of time
    //! elapsed since the last call to reset() is greater than the top
    //! value.
    class Counter
    {
    public:
      //! Constructor.
      Counter():
        overflow_(false)
      {
        reset();
      }

      //! Constructor.
      //! @param top counter's top value.
      Counter(ros::Duration top):
        overflow_(false),
        top_(top.toSec())
      {
        reset();
      }

      ~Counter()
      {}

      //! Set top value.
      //! @param top counter's top value.
      inline void
      setTop(ros::Duration top)
      {
         top_ = top;
         reset();
      }

      //! Get top value.
      //! @return counter's top value.
      inline ros::Duration
      getTop(void) const
      {
        return top_;
      }

      //! Reset counter.
      inline ros::Time
      reset(void)
      {
        last_ = ros::Time::now();
        overflow_ = false;
        return last_;
      }

      //! Check if the counter has reached the top value.
      //! @return true if an overflow occured, false otherwise.
      inline bool
      overflow(void)
      {
        if (!overflow_)
          overflow_ = (ros::Time::now() >= (last_ + top_));

        return overflow_;
      }

      //! Get remaining time.
      //! @return time remaining (s).
      inline ros::Duration
      getRemaining(void) const
      {
        if (overflow_)
          return ros::Duration(0);

        return top_ - (ros::Time::now() - last_);
      }

      //! Get elapsed time.
      //! @return elapsed time (s).
      inline ros::Duration
      getElapsed(void) const
      {
        return ros::Time::now() - last_;
      }

    private:
      //! Top value.
      ros::Duration top_;
      //! Time of last reset.
      ros::Time last_;
      //! True if the counter overflowed.
      bool overflow_;
    };
  }
}

#endif