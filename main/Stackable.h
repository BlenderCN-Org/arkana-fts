/*
    Defines an interface to a stack-able context. Means that the implementer
    supports the push / pop context. The implementing class has to make sure
    that it context is saved and can be restored to that state later on. 
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

#ifndef STACKABLE_H
#define STACKABLE_H
namespace FTS {
    class Stackable
    {
    public:
        virtual void pushContext() = 0;
        virtual void popContext() = 0 ;

    protected:
        Stackable() {};
        ~Stackable() {};
    };
}
#endif // STACKABLE_H
