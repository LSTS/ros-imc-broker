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
// Author: Ricardo Martins                                                *
// Author: Paulo Dias                                                     *
//*************************************************************************

#ifndef ROS_IMC_BROKER_UTIL_STRING_HPP_INCLUDED_
#define ROS_IMC_BROKER_UTIL_STRING_HPP_INCLUDED_

// ISO C++ headers
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

namespace ros_imc_broker
{
  namespace Util
  {
    //! Blank characters.
    static const std::string c_blank = " \n\r\t";

    //!
    //! @author Paulo Dias <pdias@lsts.pt>
    class String
    {
    public:

      //! Split a string into a vector of given type.
      //! @param s string to split.
      //! @param sep separator string.
      //! @param lst vector of Type.
      template <typename Type>
      static void
      split(const std::string& s, const std::string& sep, std::vector<Type>& lst)
      {
        size_t new_i = 0; // new index
        size_t old_i = 0; // old index
        Type d;

        if (trim(s) == "")
          return;

        while (1)
        {
          // Find next separator character
          new_i = s.find(sep, old_i);

          std::stringstream sin(trim(s.substr(old_i, new_i - old_i)));

          sin >> d;

          lst.push_back(d);

          if (new_i == std::string::npos)
            break;

          old_i = new_i + 1;
        }
      }

      static
      std::string
      ltrim(const std::string& s)
      {
        size_t first = 0;
        first = s.find_first_not_of(c_blank);

        if (first != std::string::npos)
          return s.substr(first);

        // If we get here the string only has blanks.
        return "";
      }

      static
      std::string
      rtrim(const std::string& s)
      {
        size_t last = 0;

        last = s.find_last_not_of(c_blank);

        if (last != std::string::npos)
          return s.substr(0, last + 1);

        // If we get here the string only has blanks.
        return "";
      }

      static
      std::string
      trim(const std::string& s)
      {
        std::string n = ltrim(s);

        return rtrim(n);
      }

      static
      void
      toLowerCase(std::string& str)
      {
        for (unsigned int i = 0; i < str.size(); ++i)
          str[i] = std::tolower(str[i]);
      }

      static
      void
      toUpperCase(std::string& str)
      {
        for (unsigned int i = 0; i < str.size(); ++i)
          str[i] = std::toupper(str[i]);
      }

    private:

    };
  }
}

#endif