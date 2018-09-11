/**
* @file include/retdec/ctypes/namespace.h
* @brief A representation of C++ namespaces.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_CTYPES_NAMESPACE_H
#define RETDEC_CTYPES_NAMESPACE_H

#include <vector>
#include <string>

namespace retdec {
namespace ctypes {

/**
 * @brief Representation of C++ namespaces.
 */
class Namespace
{
	public:
		Namespace();
		Namespace(std::vector<std::string> names);

		std::vector<std::string> getNames();

	private:
		std::vector<std::string> names;
};

}
}

#endif //RETDEC_NAMESPACE_H
