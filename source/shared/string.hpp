#ifndef STRING_HPP
#define STRING_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

using std::string;

template < typename ... Args >
string string_format( const string &format, Args ... args )
{
	size_t size = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
	std::unique_ptr< char[] > buf = std::make_unique< char[] >( size );
	std::snprintf( buf.get(), size, format.c_str(), args ... );
	return string( buf.get(), buf.get() + size - 1 );
}

template < typename ... Args >
std::ostream &stprintf( const string &format, Args ... args )
{
	size_t size = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
	std::unique_ptr< char[] > buf = std::make_unique< char[] >( size );
	std::snprintf( buf.get(), size, format.c_str(), args ... );
	return std::cout << string( buf.get(), buf.get() + size - 1 );
}

template < typename ... Args >
std::ostream &stprintf( std::ostream &stream, const string &format, Args ... args )
{
	size_t size = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
	std::unique_ptr< char[] > buf = std::make_unique< char[] >( size );
	std::snprintf( buf.get(), size, format.c_str(), args ... );
	return stream << string( buf.get(), buf.get() + size - 1 );
}

#endif // STRING_HPP