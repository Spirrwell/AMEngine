#ifndef IINTERFACE_H
#define IINTERFACE_H

#include <string>

class IDLLInterface
{
public:
	virtual ~IDLLInterface() {}

	virtual bool Init() = 0;

    virtual const std::string &GetInterfaceName() = 0;
    virtual void *GetInterface() = 0;
};

#endif // IINTERFACE_H
