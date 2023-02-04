#pragma once

// 128-bit globally unique identifier
// Implements a hash function and operator == overlaod to allow the type to be used as the key in a map
class guid
{
private:
	// 4 32-bit components
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;

public:
	// Default constructor
	guid();

	// Access immutable individual components through index
	const uint32_t& operator[](const size_t index) const
	{
		//uint32_t* data = &a;
		return *(&a + index);
	}

	// Equality comparison
	bool operator==(const guid& other) const
	{
		return a == other.a &&
			b == other.b &&
			c == other.c &&
			d == other.d;
	}

public:
	// Generates a new guid
	void newGuid();

	// Resets the guid leaving it in its default state
	void reset();

	// Converts guid to a string representation
	std::string toString() const;
};

// Provide a hash function for the guid
namespace std
{
	template<>
	struct hash<guid>
	{
		// Hash operator() overload
		std::size_t operator()(const guid& inGuid) const
		{
			size_t res = 17;
			res = res * 31 + hash<uint32_t>()(inGuid[0]);
			res = res * 31 + hash<uint32_t>()(inGuid[1]);
			res = res * 31 + hash<uint32_t>()(inGuid[2]);
			res = res * 31 + hash<uint32_t>()(inGuid[3]);
			return res;
		}
	};
}