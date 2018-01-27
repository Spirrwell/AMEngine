#ifndef MEMORYREADER_HPP
#define MEMORYREADER_HPP

class MemoryReader
{
public:
	MemoryReader();
	MemoryReader( char *pData, size_t length, bool bAutoFree = true );

	~MemoryReader();

	void open( char *pData, size_t length, bool bAutoFree = true );
	
	void close();
	void close_and_free();

	bool seek( size_t nPos );
	bool read( char *pDst, size_t count );

	size_t pos() { return m_Pos; }
	size_t size() { return m_Size; }

	char *data() { return m_pData; }

	// End of memory?
	bool eom() { return ( m_Pos == m_Size ); }

private:
	char *m_pData;

	size_t m_Size;
	size_t m_Pos;

	bool m_bAutoFree;
};

#endif // MEMORYREADER_HPP