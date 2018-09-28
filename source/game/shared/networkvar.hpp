#ifndef NETWORKVAR_HPP
#define NATWORKVAR_HPP

#include "engine/inetworkvar.hpp"

#include <cstring>
#include <algorithm>

template < typename T >
class NetworkVar : public INetworkVar
{
public:
	NetworkVar()
	{
		m_bStateChanged = false;
		m_Size = sizeof( T );
	}

	NetworkVar &operator=( T rhs )
	{
		m_Var = rhs;
		m_bStateChanged = true;
		return *this;
	}

	operator T() const
	{
		return m_Var;
	}

	operator T&()
	{
		return m_Var;
	}

	T Get()
	{
		return m_Var;
	}

	void *data() override
	{
		return ( void * )( &m_Var );
	}

	size_t sizeofElement() override
	{
		return m_Size;
	}

	bool StateChanged() override
	{
		return m_bStateChanged;
	}

protected:
	// Size can be overriden by network table to ensure consistent size across platforms
	void SetSize( size_t size )
	{
		// Ensures size is always less than or equal to sizeof( T )
		m_Size = std::min( sizeof( T ), size );
	}

	void SetData( char *pData )
	{
		std::memcpy( ( void* )&m_Var, ( void* )pData, sizeofElement() );
		m_bStateChanged = false;
	}

private:
	T m_Var;

	bool m_bStateChanged;
	size_t m_Size;
};

#endif // NETWORKVAR_HPP