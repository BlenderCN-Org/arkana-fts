#ifndef FTS_ARKANA_RESOURCE_PROVIDER_H
#define FTS_ARKANA_RESOURCE_PROVIDER_H

#include "CEGUIBase.h"
#include "CEGUIResourceProvider.h"

#include <map>

namespace FTS {
    class Path;
}

// Start of CEGUI namespace section
namespace CEGUI
{
class CEGUIEXPORT ArkanaResourceProvider : public ResourceProvider
{
public:
	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
	ArkanaResourceProvider() {}
	virtual ~ArkanaResourceProvider() {}

    /*!
    \brief
        Set the directory associated with a given resource group identifier.

    \param resourceGroup
        The resource group identifier whose directory is to be set.

    \param directory
        The directory to be associated with resource group identifier
        \a resourceGroup

    \return
        Nothing.
    */
    void setResourceGroupDirectory(const String& resourceGroup, const String& directory);

    /*!
    \brief
        Return the directory associated with the specified resource group
        identifier.

    \param resourceGroup
        The resource group identifier for which the associated directory is to
        be returned.

    \return
        String object describing the directory currently associated with resource
        group identifier \a resourceGroup.

    \note
        This member is not defined as being const because it may cause
        creation of an 'empty' directory specification for the resourceGroup
        if the resourceGroup has not previously been accessed.
    */
    const String& getResourceGroupDirectory(const String& resourceGroup);

    /*!
    \brief
        clears any currently set directory for the specified resource group
        identifier.

    \param resourceGroup
        The resource group identifier for which the associated directory is to
        be cleared.
    */
    void clearResourceGroupDirectory(const String& resourceGroup);

    void loadRawDataContainer(const String& filename, RawDataContainer& output, const String& resourceGroup);
    void unloadRawDataContainer(RawDataContainer& data);

protected:
    /*!
    \brief
        Return the final path and filename, taking into account the given
        resource group identifier that should be used when attempting to
        load the data.
    */
    FTS::Path getFinalFilename(const String& filename, const String& resourceGroup) const;

    typedef std::map<String, String, String::FastLessCompare> ResourceGroupMap;
    ResourceGroupMap m_resourceGroups;
};

} // End of  CEGUI namespace section

#endif /* FTS_ARKANA_RESOURCE_PROVIDER_H */

 /* EOF */
