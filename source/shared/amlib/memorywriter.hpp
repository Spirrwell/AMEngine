#ifndef MEMORYWRITER_HPP
#define MEMORYWRITER_HPP

class MemoryWriter
{
public:
	// Construct with no data, just write to a buffer we create
	MemoryWriter( bool bAutoFree = true );

	// Initialize with a COPY of pre-existing data with the length specified
	MemoryWriter( char *pData, size_t length, bool bAutoFree = true );

	// Destructor, cleanup if we're told to
	virtual ~MemoryWriter();

	// Sets m_pData to nullptr, let the programmer cleanup the memory
	void close();

	// Sets m_pData to nullptr after freeing the memory
	void close_and_free();

	// Returns the data we're writing to
	char *data() { return m_pData; }

	// Writes pSrc to m_pData with the specified length in bytes
	virtual void write( char *pSrc, size_t length );

	// Returns the current size of m_pData
	size_t size() { return m_Size; }

	// TODO: Add seeking functionality for more dynamic and easy writing
	// Perhaps in a derrived class?

private:

	// The memory we're writing to
	char *m_pData;

	// The current size of m_pData
	size_t m_Size;

	// Should we automatically free m_pData in destructor?
	bool m_bAutoFree;
};

#endif // MEMORYWRITER_HPP