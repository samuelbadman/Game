#pragma once

class fileIO
{
public:
	/** Checks if the file at the path exists
	* @return True if the file exists on disk, otherwise false
	*/
	static bool fileExists(const std::string& file);

	/** Replaces the path's current extension with the new extension
	* @return A new string containing the path with the new extension
	*/
	static std::string replaceExtension(const std::string& file, const std::string& newExtension);

	/** Gets just the extension from the path
	* @return A new string containing the extension
	*/
	static std::string getExtension(const std::string& file);

	/** Removes the extension from the path
	* @return A new string containing just the path portion of the original path without the extension part
	*/
	static std::string removeExtension(const std::string& file);

	/** Gets the size of the requested file
	* @return The size of the requested file
	*/
	static uintmax_t fileSize(const std::string& file);

	/** Writes the serialized buffer to the hard disk. */
	static void writeSerializedBuffer(const std::string& file, const std::vector<uint8_t>& buffer);

	/** Reads the contents of a serialized file from the hard disk into a buffer */
	static void readSerializedBuffer(const std::string& file, std::vector<uint8_t>& outBuffer);
};