#ifndef AMVECTOR_HPP
#define AMVECTOR_HPP

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <initializer_list>

template < typename T >
class AMVector
{
public:
	AMVector()
	{
		m_pElementList = nullptr;
		m_iElementCount = 0;
		m_iVectorCapacity = 0;

		// Size of element type shouldn't change, so we store it.
		m_iElementSize = sizeof( T );
	}

	AMVector( int32_t iVectorCapacity ) : AMVector()
	{
		Reserve( iVectorCapacity );
	}

	AMVector( const AMVector &other )
	{
		m_iElementCount = other.m_iElementCount;
		m_iElementSize = other.m_iElementSize;
		m_iVectorCapacity = other.m_iVectorCapacity;

		m_pElementList = new T[ m_iVectorCapacity ];
		std::copy( &other.m_pElementList[ 0 ], &other.m_pElementList[ 0 ] + other.m_iElementCount, m_pElementList );

	}

	AMVector( std::initializer_list< T > elements )
	{
		m_iElementSize = sizeof ( T );
		m_iElementCount = ( int32_t )elements.size();
		m_iVectorCapacity = ( int32_t )elements.size();
		m_pElementList = new T[ m_iElementCount ];

		// Should maybe use std::begin( elements ) + m_iElementCount as we're casting size() which is size_t to int32_t
		std::copy( std::begin( elements ), std::end( elements ), m_pElementList );
	}

	~AMVector()
	{
		if ( m_pElementList )
			delete[] m_pElementList;
	}

	int32_t AddToTail( const T &element )
	{
		return InsertBefore( InvalidIndex(), element );
	}

	void AddMultipleToTail( T *pElements, int32_t numElements )
	{
		if ( pElements == nullptr )
			return;

		for ( int32_t i = 0; i < numElements; i++ )
			InsertBefore( InvalidIndex(), pElements[ i ] );
	}

	int32_t AddToHead( const T &element )
	{
		return InsertBefore( 0, element );
	}

	void AddMultipleToHead( T *pElements, int32_t numElements )
	{
		if ( pElements == nullptr )
			return;

		for ( int32_t i = 0; i < numElements; i++ )
			InsertBefore( 0, pElements[ i ] );
	}

	AMVector& operator = ( AMVector other )
	{
		std::swap( m_iElementCount, other.m_iElementCount );
		std::swap( m_iVectorCapacity, other.m_iVectorCapacity );
		std::swap( m_iElementSize, other.m_iElementSize );
		std::swap( m_pElementList, other.m_pElementList );

		return *this;
	}
	T& operator [] ( const int32_t &elem )
	{
		return m_pElementList[ elem ];
	}

	const T& operator [] ( const int32_t &elem ) const
	{
		return m_pElementList[ elem ];
	}

	bool IsValidIndex( int32_t index )
	{
		return ( index < Count() && Count() > 0 && index >= 0 );
	}

	T *Data() { return m_pElementList; }

	inline const int32_t &Count() { return m_iElementCount; }
	inline const int32_t &Capacity() { return m_iVectorCapacity; }

	void Purge()
	{
		if ( m_pElementList )
		{
			delete[] m_pElementList;
			m_pElementList = nullptr;
		}

		m_iElementCount = 0;
		m_iVectorCapacity = 0;
	}

	void PurgeAndDeleteElements()
	{
		for ( int32_t i = 0; i < Count(); i++ )
			if ( m_pElementList[ i ] )
				delete m_pElementList[ i ];

		Purge();
	}

	void Remove( int32_t index )
	{
		if ( !IsValidIndex( index ) ) // This also checks if element count is 0
			return;
		else if ( Count() == 1 )
		{
			Purge();
			return;
		}

		ShiftElementsLeft( index );

		int32_t iNewElementCount = Count() - 1;
		int32_t iNewVectorCapacity = Capacity() - 1;

		T *pTempElements = new T[ iNewVectorCapacity ];
		std::copy( m_pElementList, &m_pElementList[ 0 ] + Count(), pTempElements );

		delete[] m_pElementList;

		m_pElementList = pTempElements;
		m_iElementCount = iNewElementCount;
		m_iVectorCapacity = iNewVectorCapacity;
	}

	void Resize( int32_t newSize )
	{
		if ( newSize == Capacity() )
			return;
		else if ( newSize > Capacity() )
		{
			Reserve( newSize - Capacity() );
			return;
		}
		else if ( newSize < Capacity() )
		{
			T *pTempElements = new T[ newSize ];
			std::copy( m_pElementList, &m_pElementList[ 0 ] + newSize, pTempElements );

			delete[] m_pElementList;
			m_pElementList = pTempElements;

			if ( newSize < Count() )
				m_iElementCount = newSize;

			m_iVectorCapacity = newSize;

			return;
		}
	}

	void Reserve( int32_t numElements )
	{
		if ( m_pElementList == nullptr )
		{
			m_pElementList = new T[ numElements ];
			m_iVectorCapacity = numElements;
			return;
		}

		int32_t iNewCapacity = Capacity() + numElements;
		T *pTempElements = new T[ iNewCapacity ];
		std::copy( m_pElementList, &m_pElementList[ 0 ] + Count(), pTempElements );
		delete[] m_pElementList;
		m_pElementList = pTempElements;
		m_iVectorCapacity = iNewCapacity;
	}

	const size_t &GetElementSize() { return m_iElementSize; }

	bool IsEmpty()
	{
		return ( Count() == 0 );
	}

	inline int32_t InvalidIndex() { return -1; }

protected:

	// Note: Inserting before the specified index, means that the new element will be at that index because of element shifting
	// It may be better to rename this to InsertAt
	int32_t InsertBefore( int32_t index, const T &element )
	{
		// We should probably do some checking to make sure we're not hitting the max value for our element count's type
		// However I don't have 17 GB of spare RAM to test this...

		if ( index == Capacity() && index == 0 )
			index = InvalidIndex();

		// Something bad happened
		if ( index >= Capacity() )
			return InvalidIndex();

		T *pTempElements = nullptr;
		int32_t iNewElementCount = Count() + 1;

		if ( iNewElementCount > Capacity() )
		{
			pTempElements = new T[ iNewElementCount ];
			m_iVectorCapacity = iNewElementCount;

			if ( m_pElementList == nullptr )
			{
				m_pElementList = pTempElements;
				m_pElementList[ m_iElementCount ] = element;
				m_iElementCount = iNewElementCount;
				return Count() - 1;
			}

			std::copy( m_pElementList, &m_pElementList[ 0 ] + Count(), pTempElements );
			delete[] m_pElementList;

			m_pElementList = pTempElements;

			if ( index == InvalidIndex() )
			{
				m_pElementList[ m_iElementCount ] = element;
				m_iElementCount = iNewElementCount;
				return Count() - 1;
			}

			m_iElementCount = iNewElementCount;
			ShiftElementsRight( index );

			m_pElementList[ index ] = element;
			return index;
		}

		if ( index == InvalidIndex() )
		{
			m_pElementList[ m_iElementCount ] = element;
			m_iElementCount = iNewElementCount;
			return Count() - 1;
		}

		m_iElementCount = iNewElementCount;

		ShiftElementsRight( index );
		m_pElementList[ index ] = element;
		return index;
	}

	void ShiftElementsRight( int32_t index )
	{
		for ( int32_t i = Count() - 1; ( i >= index && i != 0 ); i-- )
			m_pElementList[ i ] = m_pElementList[ i - 1 ];
	}

	void ShiftElementsLeft( int32_t index )
	{
		for ( int32_t i = index; i < Count() - 1; i++ )
			m_pElementList[ i ] = m_pElementList[ i + 1 ];
	}

	int32_t m_iElementCount; // Number of elements in vector
	int32_t m_iVectorCapacity; // Number of allocated slots in vector
	size_t m_iElementSize; // Size of an individual element in vector
	T *m_pElementList; // Actual vector/element list
};

#endif // AMVECTOR_HPP
