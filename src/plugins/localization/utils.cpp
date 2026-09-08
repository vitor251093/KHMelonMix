#include "utils.h"

#include <fstream>
#include <iostream>
#include <assert.h>

#include "p2.h"
#include "cakp.h"

namespace utils {

std::vector<uint8_t> readBinaryFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cout << "Could not open file: " << filename << std::endl;
        return {};
    }

    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void saveBinaryFile(const std::vector<uint8_t>& data, const std::string& filename)
{
    if (data.size() > 0)
    {
        std::ofstream os(filename, std::ofstream::binary);
        os.write((char*)data.data(), data.size());
    }
}

ndsloc::EFileFormat getFileFormat(const void* data, std::size_t size)
{
    using namespace ndsloc;

    if (!data || size == 0)
        return EFileFormat::Empty;

    const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(data);
    const uint32_t dataSize = static_cast<uint32_t>(size);

    if (p2::isP2File(dataPtr))
        return EFileFormat::P2;

    if (cakp::isCAKP(dataPtr))
        return EFileFormat::CAKP;

    return EFileFormat::Unknown;
}

std::string getExtName(ndsloc::EFileFormat type)
{
    using namespace ndsloc;
    switch (type)
    {
    case EFileFormat::Empty: return "Empty";
    case EFileFormat::P2: return "P2";
    case EFileFormat::CAKP: return "CAKP";
    default:
        return "";
    }
}

bool ends_with(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size()
        && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void replace_in_string(std::string& str, const std::string& replace, const std::string& with)
{
    size_t index = 0;
    while (true)
    {
        index = str.find(replace, index);
        if (index == std::string::npos)
            break;

        str.replace(index, replace.size(), with);

        index += with.size();
    }
}

void replace_in_ustring(std::u16string& str, const std::u16string& replace, const std::u16string& with)
{
    size_t index = 0;
    while (true)
    {
        index = str.find(replace, index);
        if (index == std::u16string::npos)
            break;

        str.replace(index, replace.size(), with);

        index += with.size();
    }
}

uint16_t readUInt16(const uint8_t* dataPtr, uint32_t pos)
{
    return dataPtr[pos] + (dataPtr[pos + 1] << 8);
}

uint32_t readUInt24(const uint8_t* dataPtr, uint32_t pos)
{
    return dataPtr[pos] + (dataPtr[pos + 1] << 8) + (dataPtr[pos + 2] << 16);
}

uint32_t readUInt32(const uint8_t* dataPtr, uint32_t pos)
{
    return dataPtr[pos] + (dataPtr[pos + 1] << 8) + (dataPtr[pos + 2] << 16) + (dataPtr[pos + 3] << 24);
}

} // namespace Utils
