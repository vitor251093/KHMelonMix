#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace ndsloc {

class LZSSFile
{
public:
    LZSSFile(const std::string& inputPath);
    LZSSFile(const uint8_t* inputPtr, int inputSize);

    enum ECompressType : uint8_t { LZSS = 0, ONZ, Unknown };

    ECompressType getCompressionMethod() const;
    void decompress();
    void compress(ECompressType type = LZSS);

    const std::vector<uint8_t>& getConvertedData() const { return m_outputBuffer; }
    void saveToDisk(const std::string& outPath);

private:
    void decompressLzss();
    void decompressOnz();

    void compressLzss();
    void compressOnz();

    void compress_impl(ECompressType type);

    static std::pair<int, int> search(const uint8_t* data, int position, int size, int maxLength, int maxDistance);
    static void encodeToken(std::vector<uint8_t>& out, int length, int distance, ECompressType type);

    std::vector<uint8_t> m_inputBuffer;
    const uint8_t* m_inputPtr = nullptr;
    int m_inputSize = 0;

    std::vector<uint8_t> m_outputBuffer;
};

} // namespace ndsloc
