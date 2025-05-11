/* @file UpxToConsole.cpp */

#include "UpxToConsole.hpp"

namespace xo {
  namespace process {
    rp<UpxToConsole>
    UpxToConsole::make()
    {
      return new UpxToConsole();
    } /*make*/

    UpxToConsole::UpxToConsole() = default;
  } /*namespace process*/
} /*namespace xo*/
