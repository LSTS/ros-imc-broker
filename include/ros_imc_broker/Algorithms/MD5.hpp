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
// Author: Eduardo Marques (original)                                     *
//*************************************************************************

#ifndef ROS_IMC_BROKER_ALGORITHMS_MD5_HPP_INCLUDED_
#define ROS_IMC_BROKER_ALGORITHMS_MD5_HPP_INCLUDED_

// ISO C++ headers.
#include <cstdio>
#include <stdexcept>
#include <fstream>

// MD5 headers
#define MD 5
#define PROTOTYPES 1
#include "md5/global.h"
#include "md5/md5.h"

namespace ros_imc_broker
{
  namespace Algorithms
  {
    //! MD-5 Algorithm (RFC 1321).
    class MD5
    {
    public:
      // Static utility methods

      //! Compute the MD5 hash sum of a given data buffer.
      //! @param buffer data buffer.
      //! @param len data buffer length.
      //! @param digest output MD5 digest (16 bytes long).
      static void
      compute(const uint8_t* buffer, unsigned int len, uint8_t* digest)
      {
        MD5 md5;
        MD5_CTX context;

        MD5Init(&context);
        MD5Update(&context, (unsigned char*)buffer, len);
        MD5Final(digest, &context);
      }

      //! Compute the MD5 hash sum for a file.
      //! @param path file path.
      //! @param digest output MD5 digest (16 bytes long).
      static void
      compute(const char* path, uint8_t* digest)
      {
        std::ifstream ifs(path, std::ios::binary);

        if (!ifs.is_open())
          throw std::runtime_error("failed to open file");

        MD5_CTX context;
        MD5Init(&context);

        unsigned char buffer[1024];
        while (!ifs.eof())
        {
          ifs.read((char*)buffer, sizeof(buffer));
          MD5Update(&context, buffer, ifs.gcount());
        }

        MD5Final(digest, &context);
      }

      // Instance methods

      //! Constructor.
      MD5(void):
        m_handle(new MD5_HANDLE)
      {
        reset();
      }

      //! Destructor.
      ~MD5(void)
      {
        delete m_handle;
      }

      //! Reset to an initial state.
      void
      reset(void)
      {
        MD5Init(&m_handle->ctx);
      }

      //! Update MD5 hash with input data.
      //! @param data data buffer
      //! @param size length of data
      void
      update(const uint8_t* data, int size)
      {
        MD5Update(&m_handle->ctx, (uint8_t*)data, size);
      }

      //! Finalize MD5 hash and obtain digest.
      //! @param digest output digest (16 bytes long)
      void
      finalize(uint8_t* digest)
      {
        MD5Final(digest, &m_handle->ctx);
      }

    private:
      // Forward declaration.
      // You cannot forward declare a type defined from an anonymous structure,
      // as is the case of MD5_CTX, hence the slightly odd construction below.
      // (harmless in principle).
      struct MD5_HANDLE
      {
        ::MD5_CTX ctx;
      };

      //! Private handle for MD5 calculation.
      MD5_HANDLE* m_handle;
    };
  }
}

#endif
