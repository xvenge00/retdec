/**
* @file src/ctypes/namespace.h
* @brief Implementation of C++ namespace representation.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include "retdec/ctypes/namespace.h"

namespace retdec {
namespace ctypes {

Namespace::Namespace() : names() {}

Namespace::Namespace(std::vector<std::string> names) : names(names) {}

std::vector<std::string> Namespace::getNames() { return names; }

}
}
