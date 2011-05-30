#include "main.h"

#include "ArkanaResourceProvider.h"
#include <CEGUIExceptions.h>

#include <fstream>
#include <iostream>

#include "dLib/dFile/dFile.h"
#include "utilities/utilities.h"
#include "dLib/dConf/configuration.h"

// Start of CEGUI namespace section
namespace CEGUI
{

    void ArkanaResourceProvider::loadRawDataContainer(const String& filename, RawDataContainer& output, const String& resourceGroup)
    {
        if (filename.empty())
        {
            throw InvalidRequestException(
                "DefaultResourceProvider::load - Filename supplied for data loading must be valid");
        }

        try {
            FTS::Path final_filename(this->getFinalFilename(filename, resourceGroup));
            FTS::File::Ptr pFile = FTS::File::open(final_filename, FTS::File::Read);

            uint8* data = new uint8[pFile->getSize()];
            pFile->readNoEndian(data, pFile->getSize());

            output.setData(data);
            output.setSize(pFile->getSize());
        } catch(const FTS::ArkanaException& e) {
            // TODO: translate to CEGUI exception.
            throw InvalidRequestException(e.what());
        }
    }

    void ArkanaResourceProvider::unloadRawDataContainer(RawDataContainer& data)
    {
        uint8* ptr = data.getDataPtr();
        delete [] ptr;
        data.setData(0);
        data.setSize(0);
    }

    void ArkanaResourceProvider::setResourceGroupDirectory(const String& resourceGroup, const String& directory)
    {
        m_resourceGroups[resourceGroup] = directory;
    }

    const String& ArkanaResourceProvider::getResourceGroupDirectory(const String& resourceGroup)
    {
        return m_resourceGroups[resourceGroup];
    }

    void ArkanaResourceProvider::clearResourceGroupDirectory(const String& resourceGroup)
    {
        ResourceGroupMap::iterator iter = m_resourceGroups.find(resourceGroup);

        if (iter != m_resourceGroups.end())
            m_resourceGroups.erase(iter);
    }

    FTS::Path ArkanaResourceProvider::getFinalFilename(const String& in_sFilename, const String& in_sResourceGroup) const
    {
        FTS::Path sNameWithGroupDir;

        // look up resource group directory
        ResourceGroupMap::const_iterator iter =
            m_resourceGroups.find(in_sResourceGroup.empty() ? d_defaultResourceGroup : in_sResourceGroup);

        // if there was an entry for this group, use it's directory as the
        // first part of the filename
        if (iter != m_resourceGroups.end())
            sNameWithGroupDir = FTS::String(iter->second);

        // append the filename part that we were passed
        sNameWithGroupDir = sNameWithGroupDir.appendWithSeparator(FTS::Path(in_sFilename));

        // For layouts, add the language to the name!
        if(in_sResourceGroup == "layouts") {
            FTS::Configuration conf ("conf.xml", FTS::ArkanaDefaultSettings());
            FTS::String sMyLang = conf.get("Language");
            FTS::Path sNameInMyLang = FTS::String(sNameWithGroupDir) + FTS::String(".") + sMyLang + FTS::String(".layout");
            FTS::Path sNameInEngl = FTS::String(sNameWithGroupDir) + FTS::String(".English.layout");

            // Check with the player's current language.
            if(FTS::File::available(sNameInMyLang, FTS::File::Read)) {
                return sNameInMyLang;
            // Or fallback to English.
            } else if(FTS::File::available(sNameInEngl, FTS::File::Read)) {
                return sNameInEngl;
            }
        }

        // return result
        return sNameWithGroupDir;
    }

} // End of  CEGUI namespace section
