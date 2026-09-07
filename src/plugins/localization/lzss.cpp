#include "lzss.h"

#include <assert.h>

#include "utils.h"

namespace ndsloc {

namespace {

const int BlockSize = 8;
const int SlidingWindowSize = 4096;
const int MinMatchLength = 3;

const int LzssMaxMatchLength = 18;
const uint8_t LzssCompressionType = 0x10;

const int OnzMaxMatchLength = 0x10110;
const uint8_t OnzCompressionType = 0x11;

}

LZSSFile::LZSSFile(const std::string& inputPath)
{
    m_inputBuffer = utils::readBinaryFile(inputPath);
    m_inputPtr = m_inputBuffer.data();
    m_inputSize = m_inputBuffer.size();
}

LZSSFile::LZSSFile(const uint8_t* inputPtr, int inputSize)
    : m_inputPtr(inputPtr)
    , m_inputSize(inputSize)
{
}

LZSSFile::ECompressType LZSSFile::getCompressionMethod() const
{
    assert(m_inputSize > 0 && m_inputPtr != nullptr);

    if (m_inputPtr[0] == LzssCompressionType)
        return ECompressType::LZSS;
    else if (m_inputPtr[0] == OnzCompressionType)
        return ECompressType::ONZ;
    else
        return ECompressType::Unknown;
}

void LZSSFile::decompress()
{
    auto algo = getCompressionMethod();
    switch (algo)
    {
    case ECompressType::LZSS:
        decompressLzss();
        break;
    case ECompressType::ONZ:
        decompressOnz();
        break;
    default: {}
        // Not implemented
    }
}


void LZSSFile::decompressLzss()
{
    int currentPosition = 4;
    int distance = 1;

    const uint8_t* compressedData = m_inputPtr;
    const int inputSize = m_inputSize;

    int outputSize = compressedData[1] + (compressedData[2] << 8) + (compressedData[3] << 16);
    m_outputBuffer.resize(outputSize);
    uint8_t* outPtrStart = m_outputBuffer.data();
    uint8_t* outPtr = outPtrStart;

    int processedBytes = 0;
    while (processedBytes <= outputSize && currentPosition < inputSize)
    {
        int forward = 1;
        for (int i = 0; i < 8; i++)
        {
            if (currentPosition + forward >= inputSize)
            {
                break;
            }

            if (processedBytes >= outputSize)
            {
                break;
            }

            if (((compressedData[currentPosition] >> (7 - i)) & 1) == 1)
            {
                int amountToCopy = 3 + ((compressedData[currentPosition + forward] >> 4) & 0xF);
                int copyFrom = distance + ((compressedData[currentPosition + forward] & 0xF) << 8)
                    + compressedData[currentPosition + forward + 1];
                int copyPosition = processedBytes - copyFrom;
                for (int u = 0; u < amountToCopy; u++)
                {
                    const int readPosition = copyPosition + (u % copyFrom);
                    if (readPosition >= 0 && readPosition < processedBytes)
                    {
                        *outPtr = outPtrStart[readPosition];
                        outPtr++;
                        processedBytes++;
                    }
                    else
                    {
                        return;
                    }
                }

                forward += 2;
            }
            else
            {
                *outPtr = compressedData[currentPosition + forward];
                outPtr++;

                processedBytes++;
                forward++;
            }
        }

        currentPosition += forward;
    }

    while (processedBytes < outputSize)
    {
        *outPtr = 0;
        outPtr++;

        processedBytes++;
    }
}

void LZSSFile::decompressOnz()
{
    int currentPosition = 4;

    const uint8_t* compressedData = m_inputPtr;
    const int inputSize = m_inputSize;

    int outputSize = compressedData[1] + (compressedData[2] << 8) + (compressedData[3] << 16);
    m_outputBuffer.resize(outputSize);
    uint8_t* outPtrStart = m_outputBuffer.data();
    uint8_t* outPtr = outPtrStart;

    int processedBytes = 0;
    while (processedBytes <= outputSize && currentPosition < inputSize)
    {
        int forward = 1;
        for (int i = 0; i < 8; i++)
        {
            if (currentPosition + forward >= inputSize)
            {
                break;
            }

            if (processedBytes >= outputSize)
            {
                break;
            }

            if (((compressedData[currentPosition] >> (7 - i)) & 1) == 1)
            {
                int amountToCopy;
                int copyFrom;
                uint8_t byte1 = compressedData[currentPosition + forward];
                uint8_t byte2 = compressedData[currentPosition + forward + 1];

                if ((byte1 & 0xF0) == 0x10)
                {
                    uint8_t byte3 = compressedData[currentPosition + forward + 2];
                    uint8_t byte4 = compressedData[currentPosition + forward + 3];

                    amountToCopy = 0x111 + ((byte1 & 0xF) << 12) + (byte2 << 4) + (byte3 >> 4);
                    copyFrom = 1 + ((byte3 & 0xF) << 8) + byte4;
                    forward += 4;
                }
                else if ((byte1 & 0xF0) == 0x00)
                {
                    uint8_t byte3 = compressedData[currentPosition + forward + 2];

                    amountToCopy = 0x11 + ((byte1 & 0xF) << 4) + (byte2 >> 4);
                    copyFrom = 1 + ((byte2 & 0xF) << 8) + byte3;
                    forward += 3;
                }
                else
                {
                    amountToCopy = 0x1 + (byte1 >> 4);
                    copyFrom = 1 + ((byte1 & 0xF) << 8) + byte2;
                    forward += 2;
                }

                int copyPosition = processedBytes - copyFrom;
                for (int u = 0; u < amountToCopy; u++)
                {
                    const int readPosition = copyPosition + (u % copyFrom);
                    if (readPosition >= 0 && readPosition < processedBytes)
                    {
                        *outPtr = outPtrStart[readPosition];
                        outPtr++;
                        processedBytes++;
                    }
                    else
                    {
                        return;
                    }
                }
            }
            else
            {
                *outPtr = compressedData[currentPosition + forward];
                outPtr++;

                processedBytes++;
                forward++;
            }
        }

        currentPosition += forward;
    }

    while (processedBytes < outputSize)
    {
        *outPtr = 0;
        outPtr++;

        processedBytes++;
    }
}

void LZSSFile::saveToDisk(const std::string& outPath)
{
    utils::saveBinaryFile(m_outputBuffer, outPath);
}

void LZSSFile::compress(ECompressType type)
{
    switch (type)
    {
    case ECompressType::LZSS:
        compressLzss();
        break;
    case ECompressType::ONZ:
        compressOnz();
        break;
    default:
        assert(false);
    }
}

void LZSSFile::compressLzss()
{
    assert(m_inputSize > 0 && m_inputPtr != nullptr);

    compress_impl(ECompressType::LZSS);
}

void LZSSFile::compressOnz()
{
    assert(m_inputSize > 0 && m_inputPtr != nullptr);

    compress_impl(ECompressType::ONZ);
}

std::pair<int, int> LZSSFile::search(const uint8_t* data, int position, int size, int maxLength, int maxDistance)
{
    const int remaining = size - position;
    if (remaining <= 0)
        return { 0, 0 };

    maxLength = std::min(maxLength, remaining);

    const int start = std::max(0, position - maxDistance);

    int bestDistance = 0;
    int bestLength = 0;

    for (int candidate = start; candidate < position; candidate++)
    {
        const int distance = position - candidate;

        int length = 0;
        while (length < maxLength && data[candidate + (length % distance)] == data[position + length])
        {
            length++;
        }

        if (length > bestLength)
        {
            bestLength = length;
            bestDistance = distance;

            if (bestLength == maxLength)
            {
                break;
            }
        }
    }

    return { bestDistance, bestLength };
}

void LZSSFile::encodeToken(std::vector<uint8_t>& out, int length, int distance, ECompressType type)
{
    const int offset = distance - 1;

    if (type == ECompressType::LZSS)
    {
        out.push_back((uint8_t)(((length - 3) << 4) | ((offset >> 8) & 0xF)));
        out.push_back((uint8_t)(offset & 0xFF));
        return;
    }

    if (length <= 0x10)
    {
        out.push_back((uint8_t)(((length - 1) << 4) | ((offset >> 8) & 0xF)));
        out.push_back((uint8_t)(offset & 0xFF));
    }
    else if (length <= 0x110)
    {
        const int biasedLength = length - 0x11;
        out.push_back((uint8_t)((biasedLength >> 4) & 0xF));
        out.push_back((uint8_t)(((biasedLength & 0xF) << 4) | ((offset >> 8) & 0xF)));
        out.push_back((uint8_t)(offset & 0xFF));
    }
    else
    {
        const int biasedLength = length - 0x111;
        out.push_back((uint8_t)(0x10 | ((biasedLength >> 12) & 0xF)));
        out.push_back((uint8_t)((biasedLength >> 4) & 0xFF));
        out.push_back((uint8_t)(((biasedLength & 0xF) << 4) | ((offset >> 8) & 0xF)));
        out.push_back((uint8_t)(offset & 0xFF));
    }
}

void LZSSFile::compress_impl(ECompressType type)
{
    const uint8_t* uncompressedData = m_inputPtr;
    const int inputSize = m_inputSize;

    const int maxMatchLength = (type == ECompressType::ONZ) ? OnzMaxMatchLength : LzssMaxMatchLength;
    const uint8_t compressionType = (type == ECompressType::ONZ) ? OnzCompressionType : LzssCompressionType;

    m_outputBuffer.clear();

    // Header
    m_outputBuffer.push_back(compressionType);
    m_outputBuffer.push_back(inputSize & 0xff);
    m_outputBuffer.push_back((inputSize >> 8) & 0xff);
    m_outputBuffer.push_back((inputSize >> 16) & 0xff);

    std::vector<uint8_t> blockContent;
    int position = 0;

    while (position < inputSize)
    {
        blockContent.clear();
        uint8_t blockFlags = 0;

        for (int i = BlockSize - 1; i >= 0 && position < inputSize; i--)
        {
            const std::pair<int, int> match = search(uncompressedData, position, inputSize, maxMatchLength, SlidingWindowSize);

            const int distance = match.first;
            const int length = match.second;

            if (length >= MinMatchLength)
            {
                blockFlags |= (uint8_t)(1 << i);
                encodeToken(blockContent, length, distance, type);
                position += length;
            }
            else
            {
                blockContent.push_back(uncompressedData[position]);
                position++;
            }
        }

        m_outputBuffer.push_back(blockFlags);
        m_outputBuffer.insert(m_outputBuffer.end(), blockContent.begin(), blockContent.end());
    }
}

} // namespace ndsloc
