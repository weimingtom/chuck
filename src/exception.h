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
#ifndef _EXCEPT_H
#define _EXCEPT_H
#include <setjmp.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "util/list.h"
#include "exceptionno.h"
#include "log.h"

typedef struct
{
    listnode node;
    char  info[1024];
}callstack_frame;

typedef struct
{
    listnode     node;
    sigjmp_buf   jumpbuffer;
    uint32_t     exception;
    int32_t      line; 
    const char   *file;
    const char   *func;
    void         *addr;
    int8_t       is_process;
    list         call_stack;
	
}exception_frame;

typedef struct
{
	list  expstack;
	list  csf_pool;   
}exception_perthd_st;

extern pthread_key_t g_exception_key;
#ifndef __GNUC__
extern pthread_once_t g_exception_key_once;
void exception_once_routine();
#endif

static inline void clear_callstack(exception_frame *frame)
{
    exception_perthd_st *epst = (exception_perthd_st*)pthread_getspecific(g_exception_key);
	while(list_size(&frame->call_stack) != 0)
		list_pushback(&epst->csf_pool,list_pop(&frame->call_stack));
}

static inline void print_call_stack(exception_frame *frame)
{

    if(!frame)return;
    char buf[MAX_LOG_SIZE];
    char *ptr = buf;
    int32_t size = 0;
    listnode *node = list_begin(&frame->call_stack);
    int f = 0;
    if(frame->exception == except_segv_fault)
	    size += snprintf(ptr,MAX_LOG_SIZE," exception\n %s (invaild access addr:%p)\n",exception_description(frame->exception),frame->addr);
    else
	    size += snprintf(ptr,MAX_LOG_SIZE," exception\n %s\n",exception_description(frame->exception));
    ptr = buf+size;
    while(node != NULL && size < MAX_LOG_SIZE)
    {
        callstack_frame *cf = (callstack_frame*)node;
        size += snprintf(ptr,MAX_LOG_SIZE-size,"        % 2d: %s",++f,cf->info);
        ptr = buf+size;
        node = node->next;
    }
    printf("%s",buf);
    SYS_LOG(LOG_ERROR,"%s",buf);
}

#define PRINT_CALL_STACK print_call_stack(&frame)


static inline list *get_current_thd_exceptionstack()
{
	exception_perthd_st *epst;
	callstack_frame *call_frame;
	int32_t i;
#ifndef __GNUC__	
              pthread_once(&g_exception_key_once,exception_once_routine);
#endif
	epst = (exception_perthd_st *)pthread_getspecific(g_exception_key);
	if(!epst)
	{
		epst = calloc(1,sizeof(*epst));
		for(i = 0;i < 256; ++i){
			call_frame = calloc(1,sizeof(*call_frame));
			list_pushfront(&epst->csf_pool,&call_frame->node);
		}
        pthread_setspecific(g_exception_key,epst);
	}
	return &epst->expstack;
}

static inline void expstack_push(exception_frame *frame)
{
    list *expstack = get_current_thd_exceptionstack();
	list_pushfront(expstack,&frame->node);
}

static inline exception_frame* expstack_pop()
{
    list *expstack = get_current_thd_exceptionstack();
    return (exception_frame*)list_pop(expstack);
}

static inline exception_frame* expstack_top()
{
    list *expstack = get_current_thd_exceptionstack();
    return (exception_frame*)list_begin(expstack);
}

extern void exception_throw(int32_t code,const char *file,const char *func,int32_t line,siginfo_t* info);

#define TRY do{\
	exception_frame  frame;\
    frame.node.next = NULL;\
    frame.file = __FILE__;\
    frame.func = __FUNCTION__;\
    frame.exception = 0;\
    frame.is_process = 1;\
    list_init(&frame.call_stack);\
    expstack_push(&frame);\
    int savesigs= SIGSEGV | SIGBUS | SIGFPE;\
	if(sigsetjmp(frame.jumpbuffer,savesigs) == 0)
	
#define THROW(EXP) exception_throw(EXP,__FILE__,__FUNCTION__,__LINE__,NULL)

#define CATCH(EXP) else if(!frame.is_process && frame.exception == EXP?\
                        frame.is_process=1,frame.is_process:frame.is_process)

#define CATCH_ALL else if(!frame.is_process?\
                        frame.is_process=1,frame.is_process:frame.is_process)

#define ENDTRY if(!frame.is_process)\
                    exception_throw(frame.exception,frame.file,frame.func,frame.line,NULL);\
               else {\
                    exception_frame *frame = expstack_pop();\
                    clear_callstack(frame);\
                }\
			}while(0);					

#define RETURN  do{exception_frame *top;\
                    while((top = expstack_top())!=NULL){\
                        if(strcmp(top->file,__FILE__) == 0 && strcmp(top->func,__FUNCTION__) == 0)\
                        {\
                            exception_frame *frame = expstack_pop();\
                            clear_callstack(frame);\
                        }else\
                        break;\
                    };\
                }while(0);return			

#define EXPNO frame.exception


#endif