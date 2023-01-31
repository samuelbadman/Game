#include "pch.h"
#include "mathLibrary.h"

// Todo: Create the random engine and device per thread that needs to generate random numbers
static std::random_device randomDevice;
static std::mt19937 randomEngine(randomDevice());
static std::uniform_int_distribution<uint32_t> uniformUInt32Distribution;

uint32_t mathLibrary::randomUInt32()
{
    return uniformUInt32Distribution(randomEngine);
}

float mathLibrary::fRoundTo1DecimalPlace(const float value, const bool negativeAwayFromZero)
{
    return static_cast<float>(static_cast<int32_t>(value * 10 + (0.5f * (negativeAwayFromZero ? sign(value) : 1.0f)))) / 10;
}

float mathLibrary::fRoundTo2DecimalPlace(const float value, const bool negativeAwayFromZero)
{
    return static_cast<float>(static_cast<int32_t>(value * 100 + (0.5f * (negativeAwayFromZero ? sign(value) : 1.0f)))) / 100;
}

float mathLibrary::fRoundTo3DecimalPlace(const float value, const bool negativeAwayFromZero)
{
    return static_cast<float>(static_cast<int32_t>(value * 1000 + (0.5f * (negativeAwayFromZero ? sign(value) : 1.0f)))) / 1000;
}

double mathLibrary::roundTo1DecimalPlace(const double value, const bool negativeAwayFromZero)
{
    return static_cast<double>(static_cast<int64_t>(value * 10 + (0.5 * (negativeAwayFromZero ? sign(value) : 1.0)))) / 10;
}

double mathLibrary::roundTo2DecimalPlace(const double value, const bool negativeAwayFromZero)
{
    return static_cast<double>(static_cast<int64_t>(value * 100 + (0.5 * (negativeAwayFromZero ? sign(value) : 1.0)))) / 100;
}

double mathLibrary::roundTo3DecimalPlace(const double value, const bool negativeAwayFromZero)
{
    return static_cast<double>(static_cast<int64_t>(value * 1000 + (0.5 * (negativeAwayFromZero ? sign(value) : 1.0)))) / 1000;
}

float mathLibrary::fDegreesToRadians(const float degrees)
{
    return glm::radians<float>(degrees);
}

double mathLibrary::degreesToRadians(const double degrees)
{
    return glm::radians<double>(degrees);
}

float mathLibrary::fRadiansToDegrees(const float radians)
{
    return glm::degrees<float>(radians);
}

double mathLibrary::radiansToDegrees(const double radians)
{
    return glm::degrees<double>(radians);
}
