#include "pch.h"
#include "fileIO.h"
#include "platform/framework/platformMessageBox.h"

bool fileIO::fileExists(const std::string& file)
{
    return std::filesystem::exists(file);
}

std::string fileIO::replaceExtension(const std::string& file, const std::string& newExtension)
{
    if (newExtension.size() > 0 && newExtension[0] != '.')
    {
        std::string extension = '.' + newExtension;
        return std::filesystem::path(file).replace_extension(extension).string();
    }

    return std::filesystem::path(file).replace_extension(newExtension).string();
}

std::string fileIO::getExtension(const std::string& file)
{
    return std::filesystem::path(file).extension().string();
}

std::string fileIO::removeExtension(const std::string& file)
{
    return std::filesystem::path(file).replace_extension("").string();
}

uintmax_t fileIO::fileSize(const std::string& file)
{
    std::filesystem::path path(file);

    if (fileExists(file) && std::filesystem::is_regular_file(path))
    {
        return std::filesystem::file_size(path);
    }

    return uintmax_t(0);
}

void fileIO::writeSerializedBuffer(const std::string& file, const std::vector<uint8_t>& buffer)
{
    std::ofstream ofStream(file, std::ios::out | std::ios::binary);
    if (!ofStream)
    {
        platformMessageBox(eMessageLevel::error, "fileIO::writeSerializedBuffer: Failed to write serialized buffer to " + file + '.');
        return;
    }

    ofStream.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

void fileIO::readSerializedBuffer(const std::string& file, std::vector<uint8_t>& outBuffer)
{
    std::ifstream ifStream;
    ifStream.open(file.c_str(), std::ifstream::in | std::ifstream::binary);

    if (!ifStream.good())
    {
        platformMessageBox(eMessageLevel::error, "fileIO::readSerializedBuffer: Failed to read serialized buffer from " + file + '.');
        return;
    }

    uintmax_t size = fileSize(file);
    outBuffer.resize(size);
    ifStream.seekg(0, std::ios::beg);
    ifStream.read(reinterpret_cast<char*>(outBuffer.data()), size);
    ifStream.close();
}
