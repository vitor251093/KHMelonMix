#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace ndsloc {

enum EFileFormat : uint8_t
{
	Unknown = 0,
	Empty,
	P2,
	CAKP,
};

}

namespace utils {

	std::vector<uint8_t> readBinaryFile(const std::string& filename);
	void saveBinaryFile(const std::vector<uint8_t>& data, const std::string& filename);

	ndsloc::EFileFormat getFileFormat(const void* data, std::size_t size);
	std::string getExtName(ndsloc::EFileFormat type);

	bool ends_with(const std::string& str, const std::string& suffix);
	void replace_in_string(std::string& str, const std::string& replace, const std::string& with);
	void replace_in_ustring(std::u16string& str, const std::u16string& replace, const std::u16string& with);

	uint16_t readUInt16(const uint8_t* dataPtr, uint32_t pos);
	uint32_t readUInt24(const uint8_t* dataPtr, uint32_t pos);
	uint32_t readUInt32(const uint8_t* dataPtr, uint32_t pos);

} // namespace Utils
