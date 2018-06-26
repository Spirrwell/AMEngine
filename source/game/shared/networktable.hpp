#ifndef NETWORKTABLE_HPP
#define NETWORKTABLE_HPP

#include <vector>
#include <map>

#include "string.hpp"
#include "networkvar.hpp"
#include "engine/inetworktable.hpp"

class NetworkTable : public INetworkTable
{
public:

	void UpdateNetworkTable( void *pData, size_t length ) override;

	// Add network var to table
	// If numBytes is greater than 0, it will override the send/receive size of variable
	void AddNetworkVar( INetworkVar *pNetVar, size_t numBytes = 0 );

protected:
	std::vector < INetworkVar * > GetNetworkVars() override { return m_pNetworkVars; }

private:
	std::vector < INetworkVar * > m_pNetworkVars;
};

#endif // NETWORKTABLE_HPP