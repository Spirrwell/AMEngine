#ifndef INETOBJECT_HPP
#define INETOBJECT_HPP

#include <stdint.h>

struct INetObject
{
	virtual const uint32_t &GetNetIndex() = 0;
};

#endif // INETOBJECT_HPP