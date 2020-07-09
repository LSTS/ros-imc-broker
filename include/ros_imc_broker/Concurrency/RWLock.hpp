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

#ifndef ROS_IMC_BROKER_CONCURRENCY_RW_LOCK_HPP_INCLUDED_
#define ROS_IMC_BROKER_CONCURRENCY_RW_LOCK_HPP_INCLUDED_

// ISO C++ headers
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

// Boost headers.
#include <boost/thread/shared_mutex.hpp>

namespace ros_imc_broker
{
  namespace Concurrency
  {
    class RWLock
    {
    public:
      typedef boost::shared_mutex Mutex;
      typedef boost::shared_lock<Mutex> ReadLock;
      typedef boost::unique_lock<Mutex> WriteLock;
    };
  }
}

#endif