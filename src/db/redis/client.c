#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util/string.h"
#include "util/list.h"
#include "util/bytebuffer.h"
#include "packet/rawpacket.h"

typedef struct{
	listnode node;
	char    *buff;
	size_t   b;
	size_t   e;
}word;

rawpacket *convert(list *l,size_t space){
	char  tmp[32];
	char  c;
	char *end = "\r\n";
	word *w;
	buffer_writer writer;
	bytebuffer *buffer;
	rawpacket *p;	
	space += sprintf(tmp,"%u",list_size(l)) + 3;//plus head *,tail \r\n
	buffer = bytebuffer_new(space);
	buffer_writer_init(&writer,buffer,0);
	//write the head;
	c = '*';
	buffer_write(&writer,&c,sizeof(c));
	buffer_write(&writer,tmp,strlen(tmp));
	buffer_write(&writer,end,2);

	c = '$';
	while(NULL != (w = (word*)list_pop(l))){
		sprintf(tmp,"%u",w->e - w->b);
		buffer_write(&writer,&c,sizeof(c));	
		buffer_write(&writer,tmp,strlen(tmp));
		buffer_write(&writer,end,2);

		buffer_write(&writer,w->buff+w->b,w->e - w->b);
		buffer_write(&writer,end,2);

		free(w);
	}
	buffer_write(&writer,&c,sizeof(c));
	p = rawpacket_new_by_buffer(buffer,0);
	refobj_dec((refobj*)buffer);
	return p;
}


packet *build_request(const char *cmd){
	list l;
	list_init(&l);
	char  tmp[32];
	size_t len   = strlen(cmd);
	word  *w = NULL;
	size_t i,j,space;
	i = j = space = 0;
	for(; i < len; ++i){
		if(cmd[i] != ' '){
			if(!w){ 
				w = calloc(1,sizeof(*w));
				w->b = i;
				w->buff = request;
			}
		}else{
			if(w){
				//word finish
				w->e = i;
				space += (sprintf(tmp,"%u",(w->e - w->b)) + 3);//plus head $,tail \r\n
				space += (w->e - w->b) + 2;//plus tail \r\n
				list_pushback(&l,(listnode*)w);	
				w = NULL;
				--i;
			}
		}
	}
	w->e = i;
	space += (sprintf(tmp,"%u",(w->e - w->b)) + 3);//plus head $,tail \r\n
	space += (w->e - w->b) + 2;//plus tail \r\n
	list_pushback(&l,(listnode*)w);
	return (packet*)convert(&l,space);
}


