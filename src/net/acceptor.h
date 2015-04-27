/*
    Copyright (C) <2015>  <sniperHW@163.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H

#include "comm.h"

//typedef void (*accepted_callback)(int32_t fd,sockaddr_*);

typedef struct{
    handle  base;  
    void    (*callback)(int32_t fd,sockaddr_*);
}acceptor;    

handle *acceptor_new(int32_t fd);
void    acceptor_del(handle*);    


#endif