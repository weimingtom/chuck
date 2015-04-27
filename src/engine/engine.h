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
#ifndef _ENGINE_H
#define _ENGINE_H

#include <stdint.h>
#include "comm.h"    

engine *engine_new();
void    engine_del(engine*);
int32_t engine_run(engine*);
void    engine_stop(engine*);
int32_t engine_add(engine*,handle*,generic_callback);


int32_t event_add(engine*,handle*,int32_t events);
int32_t event_remove(engine*,handle*);

int32_t event_enable(engine*,handle*,int32_t events);
int32_t event_disable(engine*,handle*,int32_t events);

static inline int32_t enable_read(engine *e,handle *h){    
    return event_enable(e,h,EVENT_READ);
}

static inline int32_t disable_read(engine *e,handle *h){
    return event_disable(e,h,EVENT_READ);
}

static inline int32_t enable_write(engine *e,handle *h){   
    return event_enable(e,h,EVENT_WRITE);
}

static inline int32_t disable_write(engine *e,handle *h){
    return event_enable(e,h,EVENT_WRITE);         
}

    
#endif
