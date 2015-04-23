/*
    Copyright (C) <2014>  <huangweilook@21cn.com>

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
#ifndef _EXCEPTIONNO_H
#define _EXCEPTIONNO_H


#include <stdlib.h>

#define MAX_EXCEPTION 4096

//system define exception
enum{
	except_invaild_num =  0,   
	except_alloc_failed = 1,   
	except_list_empty,         
	except_segv_fault,       
	except_sigbus,           
	except_arith,
    system_except_end,                      
};

//user define exception
enum{
    testexception3 = system_except_end,
}

static const char* exceptions[MAX_EXCEPTION] = {
    "except_invaild_num",
    "except_alloc_failed",
    "except_list_empty",
    "except_segv_fault",
    "except_sigbus",
    "except_arith",
    "testexception3",
    NULL,
};

static inline const char *exception_description(int expno)
{
    if(expno >= MAX_EXCEPTION) return "unknow exception";
    if(exceptions[expno] == NULL) return "unknow exception";
    return exceptions[expno];
}

#endif
