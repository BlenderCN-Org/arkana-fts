/*
    Pool for the sound objects buffer id's. 
    Copyright (C) 2010  Klaus Beyer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/


#ifndef SNDOBJPOOL_H
#define SNDOBJPOOL_H
#include <AL/al.h>
#include <dLib/dString/dString.h>
#include <map>

namespace FTS {
    
class SndObjPool
{
    class PoolElement {
        public:
        int ref_count;
        ALuint ID;
        
    };
    public:
        SndObjPool() {};
        void add(String name);
        void remove(String name);
        ALuint get(String name);
        void clear();
    private:
        std::map<const String, PoolElement> m_alBuffers;
};
}
#endif // SNDOBJPOOL_H
