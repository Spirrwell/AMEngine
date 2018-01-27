#ifndef INETWORKTABLE_HPP
#define INETWORKTABLE_HPP

#include "inetworkvar.hpp"

#include <vector>
#include <map>
#include <string>

class INetworkTable
{
	friend class EngineServer;
	friend class EngineClient;
public:
	virtual ~INetworkTable() {}

	virtual void UpdateNetworkTable( void *pData, size_t length ) = 0;

protected:
	virtual std::vector< INetworkVar * > GetNetworkVars() = 0;
};

/*class INetObject
{
public:
	virtual ~INetObject() {}

	virtual unsigned int GetNetIndex() = 0;
	virtual std::vector < INetworkVar * > GetNetVars() = 0;
	virtual bool StateChanged() = 0;
};

class INetworkTable
{
public:
	virtual ~INetworkTable() {}

	virtual bool StateChanged() = 0;
	virtual std::vector < INetObject * > GetNetObjects() = 0;
	virtual const std::string &GetName() = 0;
};*/

/*struct StateChange
{
	void *data;
	unsigned int datalength;
};

StateChange BuildNetworkUpdate()
{
	INetworkTable *pTable = nullptr;

	if ( pTable->StateChanged() )
	{
		std::string tableName = pTable->GetName();

		unsigned int totalSize = 0;
		std::vector < INetObject * > objects = pTable->GetNetObjects();
		std::vector < INetObject* > changedObjects;

		for ( auto &netObject : objects )
			if ( netObject->StateChanged() )
				changedObjects.push_back( netObject );

		unsigned int numChangedObjects = changedObjects.size();
		totalSize += sizeof ( numChangedObjects );
		char *pData = ( char* )malloc( totalSize );
		char *pBuffer = nullptr;
		memcpy( pData, &numChangedObjects, sizeof( numChangedObjects ) );

		for ( auto &netObject : changedObjects )
		{
			std::vector < INetworkVar * > netVars = netObject->GetNetVars();
			std::vector < INetworkVar * > changedVars;
			std::vector < unsigned int > changedVarsIndices;

			for ( unsigned int i = 0; i < netVars.size(); i++ )
			{
				auto &netVar = netVars[i];
				if ( netVar->StateChanged() )
				{
					changedVars.push_back( netVar );
					changedVarsIndices.push_back( i );
				}
			}

			unsigned int netIndex = netObject->GetNetIndex();
			pBuffer = ( char* )realloc( pData, totalSize + sizeof( netIndex ) );
			pData = &pBuffer[ totalSize ];
			totalSize += sizeof( netIndex );
			memcpy( pData, &netIndex, sizeof ( netIndex ) );

			unsigned int numChangedVars = changedVars.size();
			pData = ( char* )realloc( pBuffer, totalSize + sizeof( numChangedVars ) );
			pBuffer = &pData[ totalSize ];
			totalSize += sizeof( numChangedVars );
			memcpy( pBuffer, &numChangedVars, sizeof( numChangedVars ) );

			for ( auto &netVar : changedVars )
			{
				size_t size = netVar->sizeofElement();
				pBuffer = ( char* )realloc( pData, totalSize + sizeof( size ) );
				pData = &pBuffer[ totalSize ];
				memcpy( pData, &size, sizeof( size ) );
				totalSize += sizeof( size );
				pData = ( char* )realloc( pBuffer, totalSize + size );
				pBuffer = &pData[ totalSize ];
				totalSize += size;
				memcpy( pBuffer, netVar->data(), size );
				pBuffer = nullptr;
			}
		}

		StateChange change = { pData, totalSize }
	}
}*/

#endif // INETWORKTABLE_HPP