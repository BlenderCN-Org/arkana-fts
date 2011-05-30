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

#include "sndobjpool.h"
#include "fts_Snd.h"
#include "SndFile.h"
#include "assert.h"

namespace FTS {
    void SndObjPool::add(String name)
    {
        if( m_alBuffers.find(name) == m_alBuffers.end() ) {
            PoolElement element;
            element.ID = SndFile(name);
            element.ref_count = 1;
            m_alBuffers[name] = element;
        } else {
            m_alBuffers[name].ref_count++;
        }
    }

    void SndObjPool::remove(String name)
    {
        assert( m_alBuffers.find(name) != m_alBuffers.end() ) ;
        m_alBuffers[name].ref_count--;
        if( m_alBuffers[name].ref_count == 0 ) {
            SndFile::deleteBuffer(m_alBuffers[name].ID);
            m_alBuffers.erase(name);
        }
    }

    ALuint SndObjPool::get(String name)
    {
        if( m_alBuffers.find(name) == m_alBuffers.end() ) {
            return AL_NONE;
        }
        return m_alBuffers[name].ID;
    }

    void SndObjPool::clear()
    {
        m_alBuffers.clear();
    }
}
