////////////////////////////////////////////////////////////
//
// Bouge - Modern and flexible skeletal animation library
// Copyright (C) 2010 Lucas Beyer (pompei2@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////
#include <bouge/Loader.hpp>

#include <sstream>
#include <fstream>

namespace bouge
{

    Loader::Loader()
    { }

    Loader::~Loader()
    { }

    CoreMeshPtr Loader::loadMesh(const std::string& sFileName)
    {
        return CoreMeshPtr();
    }

    CoreSkeletonPtr Loader::loadSkeleton(const std::string& sFileName)
    {
        return CoreSkeletonPtr();
    }

    std::vector<CoreMaterialPtr> Loader::loadMaterial(const std::string& sFileName)
    {
        return std::vector<CoreMaterialPtr>();
    }

    std::vector<CoreMaterialSetPtr> Loader::loadMaterialSet(const std::string& sFileName)
    {
        return std::vector<CoreMaterialSetPtr>();
    }

    std::vector<CoreAnimationPtr> Loader::loadAnimation(const std::string& sFileName)
    {
        return std::vector<CoreAnimationPtr>();
    }

}
