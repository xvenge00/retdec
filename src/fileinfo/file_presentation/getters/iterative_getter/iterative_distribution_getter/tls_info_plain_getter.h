/**
 * @file src/fileinfo/file_presentation/getters/iterative_getter/iterative_distribution_getter/tls_info_plain_getter.h
 * @brief Definition of TlsInfoPlainGetter class.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#ifndef FILEINFO_FILE_PRESENTATION_GETTERS_ITERATIVE_GETTER_ITERATIVE_DISTRIBUTION_GETTER_TLS_INFO_PLAIN_GETTER_H
#define FILEINFO_FILE_PRESENTATION_GETTERS_ITERATIVE_GETTER_ITERATIVE_DISTRIBUTION_GETTER_TLS_INFO_PLAIN_GETTER_H

#include "fileinfo/file_presentation/getters/iterative_getter/iterative_distribution_getter/iterative_distribution_getter.h"

namespace fileinfo {

/**
 * Getter for TLS information
 */
class TlsInfoPlainGetter : public IterativeDistributionGetter
{
	protected:
		virtual bool loadRecord(std::size_t structIndex, std::size_t recIndex, std::vector<std::string> &record) override;
	public:
		TlsInfoPlainGetter(FileInformation &fileInfo);
		virtual ~TlsInfoPlainGetter() override;

		virtual std::size_t getBasicInfo(std::size_t structIndex, std::vector<std::string> &desc, std::vector<std::string> &info) const override;
		virtual bool getFlagDescriptors(std::size_t structIndex, std::vector<std::string> &desc, std::vector<std::string> &abbv) const override;
};

} // namespace fileinfo

#endif
