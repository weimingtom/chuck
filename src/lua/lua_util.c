#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "lua_util.h"

static inline int 
__traceback (lua_State *L) 
{
  const char *msg = lua_tostring(L, 1);
  if(msg)
    luaL_traceback(L, L, msg, 1);
  else if (!lua_isnoneornil(L, 1)) {  /* is there an error object? */
    if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
      lua_pushliteral(L, "(no error message)");
  }
  return 1;
}    

static __thread char lua_errmsg[4096];

const char*
luacall(lua_State *L,const char *fmt,...)
{
	va_list vl;
	int32_t ret,narg,nres,i,size,base;
	const char *errmsg = NULL;
	lua_State *mL;
	lua_rawgeti(L,  LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
	mL = lua_tothread(L,-1);
	lua_pop(L,1);
	if(L != mL) L = mL;	//确保L是主线程
	va_start(vl,fmt);
	size = fmt?strlen(fmt):0;
	//压入参数
	for(narg=0; narg < size; ++narg){
		switch(*fmt++){
			case 'i':{lua_pushinteger(L,va_arg(vl,lua_Integer));break;}
			case 's':{
						char *str = va_arg(vl,char*);
						if(str) lua_pushstring(L,str);
						else lua_pushnil(L);
						break;
			}
			case 'S':{
				char *str = va_arg(vl,char*);
				assert(str);
				lua_pushlstring(L,str,va_arg(vl,size_t));
				break;
			}
			case 'n':{lua_pushnumber(L,va_arg(vl,lua_Number));break;}
			case 'p':{
						void *ptr = va_arg(vl,void*);
						if(ptr) lua_pushlightuserdata(L,ptr);
						else lua_pushnil(L);
						break;
			}
			case 'r':{
				luaRef ref = va_arg(vl,luaRef);
				lua_rawgeti(L,LUA_REGISTRYINDEX,ref.rindex);
				break;
			}
			case 'f':{
				luaPushFunctor *functor = va_arg(vl,luaPushFunctor*);
				functor->Push(L,functor);
				break;
			}
			case ':':{goto arg_end;}
			default:{
				snprintf(lua_errmsg,4096,"invaild operation(%c)",*fmt);
				errmsg = lua_errmsg;
				goto end;
			}
		}
	}
arg_end:	
	nres = fmt?strlen(fmt):0;
	//插入错误处理函数	
	base = lua_gettop(L) - narg;
	lua_pushcfunction(L, __traceback);
	lua_insert(L,base);	
	ret = lua_pcall(L,narg,nres,base);
	lua_remove(L,base);
	if(ret){
		strncpy(lua_errmsg,lua_tostring(L,-1),4096);
		return lua_errmsg;
	}else if(nres){
		i = 1;
		for(;nres > 0; --nres,++i){
			switch(*fmt++){
				case 'i':{
					*va_arg(vl,lua_Integer*) = lua_tointeger(L,i);
					break;
				}
				case 's':{
					*va_arg(vl,char**) = (char*)lua_tostring(L,i);
					break;
				}
				case 'S':{
					size_t l;
					*va_arg(vl,char**) = (char*)lua_tolstring(L,i,&l);
					*va_arg(vl,size_t*) = l;
					break;
				}
				case 'n':{
					*va_arg(vl,lua_Number*) = lua_tonumber(L,i);
					break;
				}
				case 'p':{					
					*va_arg(vl,void**) = lua_touserdata(L,i);
					break;
				}
				case 'r':{
					lua_pushvalue(L,i);					
					luaRef* ref = va_arg(vl,luaRef*);
					ref->rindex = luaL_ref(L,LUA_REGISTRYINDEX);  
					lua_rawgeti(L,  LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
					ref->L = lua_tothread(L,-1);
					lua_pop(L,1);
					break;
				}
				default:{
					snprintf(lua_errmsg,4096,"invaild operation(%c)",*fmt);
					errmsg = lua_errmsg;
					goto end;					
				}
			}			
		}
	}
end:	
	va_end(vl);
	return errmsg;
}



const char*
LuaRef_Get(luaRef tab,const char *fmt,...)
{
	int32_t i,size,oldtop,k,v;
	va_list vl;
	const char *errmsg = NULL;	
	lua_State *L = tab.L;		

	assert(tab.L);
	assert(fmt);
	assert(tab.rindex != LUA_REFNIL);

	va_start(vl,fmt);
   	oldtop = lua_gettop(L);
	lua_rawgeti(L,LUA_REGISTRYINDEX,tab.rindex);
	if(!lua_istable(L,-1)){
		snprintf(lua_errmsg,4096,"arg1 is not a lua table");		
		errmsg = lua_errmsg;
		goto end;	
	}
	size = strlen(fmt);
	if(size < 2){
		snprintf(lua_errmsg,4096,"fmt invaild(kvkvkv...)");
		errmsg = lua_errmsg;
		goto end;		
	}
	for(i = 0; i < size; i += 2){	
		k = i;	
		switch(fmt[k]){
			case 'i':{lua_pushinteger(L,va_arg(vl,lua_Integer));break;}
			case 's':{lua_pushstring(L,va_arg(vl,char*));break;}
			case 'S':{
				char *str = va_arg(vl,char*);
				lua_pushlstring(L,str,va_arg(vl,size_t));
				break;
			}
			case 'n':{lua_pushnumber(L,va_arg(vl,lua_Number));break;}
			case 'p':{lua_pushlightuserdata(L,va_arg(vl,void*));break;}
			case 'r':{
				luaRef ref = va_arg(vl,luaRef);
				lua_rawgeti(L,LUA_REGISTRYINDEX,ref.rindex);
				break;
			}
			default:{
				snprintf(lua_errmsg,4096,"invaild operation(%c)",fmt[k]);
				errmsg = lua_errmsg;
				goto end;	
			}
		}
		
		lua_gettable(L,-2);	
		//get value
		v = k + 1;
		switch(fmt[v]){
			case 'i':{
				*va_arg(vl,lua_Integer*) = lua_tointeger(L,-1);
				break;
			}
			case 's':{
				*va_arg(vl,char**) = (char*)lua_tostring(L,-1);
				break;
			}
			case 'S':{
				size_t l;
				*va_arg(vl,char**) = (char*)lua_tolstring(L,-1,&l);
				*va_arg(vl,size_t*) = l;
				break;
			}
			case 'n':{
				*va_arg(vl,lua_Number*) = lua_tonumber(L,-1);
				break;
			}
			case 'p':{					
				*va_arg(vl,void**) = lua_touserdata(L,-1);
				break;
			}
			case 'r':{
				lua_pushvalue(L,-1);					
				luaRef* ref = va_arg(vl,luaRef*);
				ref->rindex = luaL_ref(L,LUA_REGISTRYINDEX);
				lua_rawgeti(L,  LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
				ref->L = lua_tothread(L,-1);
				lua_pop(L,1);
				break;
			}
			default:{
				snprintf(lua_errmsg,4096,"invaild operation(%c)",fmt[v]);
				errmsg = lua_errmsg;
				goto end;					
			}
		}
		lua_pop(L,1);//pop the value
	}					
end:
	lua_settop(L,oldtop);
	va_end(vl);
	return errmsg;
}

const char*
LuaRef_Set(luaRef tab,const char *fmt,...)
{
	assert(tab.L);
	assert(fmt);
	assert(tab.rindex != LUA_REFNIL);
	int32_t i,oldtop,size,k,v;
	va_list vl;
	const char *errmsg = NULL;	
	lua_State *L = tab.L;
	va_start(vl,fmt);
    oldtop = lua_gettop(L);
	lua_rawgeti(L,LUA_REGISTRYINDEX,tab.rindex);
	if(!lua_istable(L,-1)){
		snprintf(lua_errmsg,4096,"arg1 is not a lua table");		
		errmsg = lua_errmsg;
		goto end;	
	}
	size = strlen(fmt);
	if(size < 2){
		snprintf(lua_errmsg,4096,"fmt invaild(kvkvkv...)");
		errmsg = lua_errmsg;
		goto end;		
	}
	for(i = 0; i < size; i += 2){
	   	//push key
	   	k = i;	
		switch(fmt[k]){
			case 'i':{lua_pushinteger(L,va_arg(vl,lua_Integer));break;}
			case 's':{lua_pushstring(L,va_arg(vl,char*));break;}
			case 'S':{
				char *str = va_arg(vl,char*);
				lua_pushlstring(L,str,va_arg(vl,size_t));
				break;
			}
			case 'n':{lua_pushnumber(L,va_arg(vl,double));break;}
			case 'p':{
					void *lud = va_arg(vl,void*);
					if(lud)
						lua_pushlightuserdata(L,lud);
					else
						lua_pushnil(L);
					break;
				}
			case 'r':{
				luaRef ref = va_arg(vl,luaRef);
				lua_rawgeti(L,LUA_REGISTRYINDEX,ref.rindex);
				break;
			}
			default:{
				snprintf(lua_errmsg,4096,"invaild operation(%c)",fmt[k]);
				errmsg = lua_errmsg;
				goto end;	
			}
		}
		//push value
		v = k + 1;
		switch(fmt[v]){
			case 'i':{lua_pushinteger(L,va_arg(vl,lua_Integer));break;}
			case 's':{lua_pushstring(L,va_arg(vl,char*));break;}
			case 'S':{
				char *str = va_arg(vl,char*);
				lua_pushlstring(L,str,va_arg(vl,size_t));
				break;
			}
			case 'n':{lua_pushnumber(L,va_arg(vl,lua_Number));break;}
			case 'p':{
					void *lud = va_arg(vl,void*);
					if(lud)
						lua_pushlightuserdata(L,lud);
					else
						lua_pushnil(L);
					break;
				}
			case 'r':{
				luaRef ref = va_arg(vl,luaRef);
				lua_rawgeti(L,LUA_REGISTRYINDEX,ref.rindex);
				break;
			}
			default:{
				snprintf(lua_errmsg,4096,"invaild operation(%c)",fmt[k]);
				errmsg = lua_errmsg;
				goto end;	
			}
		}
		//set table
		lua_settable(L,-3);		
	}								
end:
	lua_settop(L,oldtop);
	va_end(vl);
	return errmsg;
}

luaRef 
makeLuaObjByStr(lua_State *L,const char *str)
{
	if(0 != luaL_dostring(L,str)){
		printf("%s\n",lua_tostring(L,-1));
		lua_pop(L,1);
		return (luaRef){.L=NULL,.rindex = LUA_REFNIL};
	}
	luaRef obj = toluaRef(L,-1);
	lua_pop(L,1);
	return obj;
}