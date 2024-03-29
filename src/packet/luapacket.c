#include "packet/luapacket.h"
#include "packet/rawpacket.h"
#include "packet/wpacket.h"
#include "packet/rpacket.h"
#include "lua/lua_util_packet.h"

#define LUARPACKET_METATABLE   "luarpacket_metatable"
#define LUAWPACKET_METATABLE   "luawpacket_metatable"
#define LUARAWPACKET_METATABLE "luarawpacket_metatable"

luapacket *
lua_topacket(lua_State *L, int index)
{
    return (luapacket*)lua_touserdata(L, index);
}

void 
lua_pushpacket(lua_State *L,packet *pk)
{
	luapacket *p = (luapacket*)lua_newuserdata(L, sizeof(*p));
	p->_packet = pk;
	if(pk->type == RPACKET)	
		luaL_getmetatable(L, LUARPACKET_METATABLE);
	else if(pk->type == WPACKET)
		luaL_getmetatable(L, LUAWPACKET_METATABLE);
	else
		luaL_getmetatable(L, LUARAWPACKET_METATABLE);
	lua_setmetatable(L, -2);
}


static int32_t 
luapacket_gc(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet){ 
		packet_del(p->_packet);
		p->_packet = NULL;
	}
    return 0;
}

static int32_t 
_write_uint8(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != WPACKET)
		return luaL_error(L,"invaild opration");
	if(lua_type(L,2) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg2");
	uint8_t v = (uint8_t)lua_tointeger(L,2);
	wpacket *wpk = (wpacket*)p->_packet;
	wpacket_write_uint8(wpk,v);	
	return 0;	
}

static int32_t 
_write_uint16(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(!p->_packet || p->_packet->type != WPACKET)
		return luaL_error(L,"invaild opration");
	if(lua_type(L,2) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg2");
	uint16_t v = (uint16_t)lua_tointeger(L,2);
	wpacket *wpk = (wpacket*)p->_packet;
	wpacket_write_uint16(wpk,v);	
	return 0;	
}

static int32_t 
_write_uint32(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(!p->_packet || p->_packet->type != WPACKET)
		return luaL_error(L,"invaild opration");
	if(lua_type(L,2) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg2");
	uint32_t v = (uint32_t)lua_tointeger(L,2);
	wpacket *wpk = (wpacket*)p->_packet;
	wpacket_write_uint32(wpk,v);	
	return 0;	
}

static int32_t 
_write_double(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(!p->_packet || p->_packet->type != WPACKET)
		return luaL_error(L,"invaild opration");
	if(lua_type(L,2) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg2");
	double v = (double)lua_tonumber(L,2);
	wpacket *wpk = (wpacket*)p->_packet;
	wpacket_write_double(wpk,v);	
	return 0;	
}

static int32_t 
_write_string(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(!p->_packet || p->_packet->type != WPACKET)
		return luaL_error(L,"invaild opration");
	if(lua_type(L,2) != LUA_TSTRING)
		return luaL_error(L,"invaild arg2");
	size_t len;
	const char *data = lua_tolstring(L,2,&len);
	wpacket *wpk = (wpacket*)p->_packet;
	wpacket_write_binary(wpk,data,len);
	return 0;	
}


int32_t 
_write_table(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(!p->_packet || p->_packet->type != WPACKET)
		return luaL_error(L,"invaild opration");	
	if(LUA_TTABLE != lua_type(L, 2))
		return luaL_error(L,"argument should be lua table");
	if(0 != lua_pack_table((wpacket*)p->_packet,L,-1))
		return luaL_error(L,"table should not hava metatable");	
	return 0;	
}


static int32_t 
_read_uint8(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = (rpacket*)p->_packet;
	lua_pushinteger(L,rpacket_read_uint8(rpk));
	return 1;	
}

static int32_t 
_read_uint16(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = (rpacket*)p->_packet;
	lua_pushinteger(L,rpacket_read_uint16(rpk));
	return 1;	
}

static int32_t 
_read_uint32(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = (rpacket*)p->_packet;
	lua_pushinteger(L,rpacket_read_uint32(rpk));
	return 1;	
}

static int32_t
_read_int8(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = (rpacket*)p->_packet;
	lua_pushinteger(L,(int8_t)rpacket_read_uint8(rpk));
	return 1;	
}

static int32_t 
_read_int16(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = (rpacket*)p->_packet;
	lua_pushinteger(L,(int16_t)rpacket_read_uint16(rpk));
	return 1;	
}

static int32_t 
_read_int32(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = (rpacket*)p->_packet;
	lua_pushinteger(L,(int32_t)rpacket_read_uint32(rpk));
	return 1;	
}


static int32_t 
_read_double(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = (rpacket*)p->_packet;
	lua_pushnumber(L,rpacket_read_double(rpk));
	return 1;	
}

static int32_t 
_read_string(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = (rpacket*)p->_packet;
	uint32_t len;
	const char *data = rpacket_read_binary(rpk,(uint16_t*)&len);
	if(data)
		lua_pushlstring(L,data,(size_t)len);
	else
		lua_pushnil(L);
	return 1;
}


static int32_t 
_read_table(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(!p->_packet || p->_packet->type != RPACKET)
		return luaL_error(L,"invaild opration");
	rpacket *rpk = 	(rpacket*)p->_packet;	
	int32_t old_top = lua_gettop(L);
	int32_t ret = lua_unpack_table(rpk,L);
	if(0 != ret){
		lua_settop(L,old_top);
		lua_pushnil(L);
	}
	return 1;
}

static int32_t 
_read_rawbin(lua_State *L)
{
	luapacket *p = lua_topacket(L,1);
	if(!p->_packet || p->_packet->type != RAWPACKET)
		return luaL_error(L,"invaild opration");
		
	rawpacket* rawpk = (rawpacket*)p->_packet;
	uint32_t len = 0;
	const char *data = rawpacket_data(rawpk,&len);
	lua_pushlstring(L,data,(size_t)len);
	return 1;						
}

static int32_t 
lua_new_wpacket(lua_State *L)
{
	int32_t argtype = lua_type(L,1); 
	if(argtype == LUA_TNUMBER || argtype == LUA_TNIL || argtype == LUA_TNONE){
		//参数为数字,构造一个初始大小为len的wpacket
		size_t len = 0;
		if(argtype == LUA_TNUMBER) len = size_of_pow2(lua_tointeger(L,1));
		if(len < 64) len = 64;
		luapacket *p = (luapacket*)lua_newuserdata(L, sizeof(*p));
		luaL_getmetatable(L, LUAWPACKET_METATABLE);
		lua_setmetatable(L, -2);
		p->_packet = (packet*)wpacket_new(len);
		return 1;			
	}else if(argtype ==  LUA_TUSERDATA){
		luapacket* other = lua_topacket(L,1);
		if(!other)
			return luaL_error(L,"invaild opration for arg1");
		if(other->_packet->type == RAWPACKET)
			return luaL_error(L,"invaild opration for arg1");
		luapacket *p = (luapacket*)lua_newuserdata(L, sizeof(*p));
		luaL_getmetatable(L, LUAWPACKET_METATABLE);
		lua_setmetatable(L, -2);
		p->_packet = make_writepacket(other->_packet);
		return 1;												
	}else	
		return luaL_error(L,"invaild opration for arg1");		
}

static int32_t 
lua_new_rpacket(lua_State *L)
{
	int32_t argtype = lua_type(L,1); 
	if(argtype ==  LUA_TUSERDATA){
		luapacket *other = lua_topacket(L,1);
		if(!other)
			return luaL_error(L,"invaild opration for arg1");
		if(other->_packet->type == RAWPACKET)
			return luaL_error(L,"invaild opration for arg1");
		luapacket *p = (luapacket*)lua_newuserdata(L, sizeof(*p));
		luaL_getmetatable(L,LUARPACKET_METATABLE);
		lua_setmetatable(L, -2);
		p->_packet = make_readpacket(other->_packet);
		return 1;					
	}else
		return luaL_error(L,"invaild opration for arg1");	
}

static int32_t 
lua_new_rawpacket(lua_State *L)
{
	int32_t argtype = lua_type(L,1); 
	if(argtype == LUA_TSTRING){
		//参数为string,构造一个函数数据data的rawpacket
		size_t len;
		char *data = (char*)lua_tolstring(L,1,&len);
		luapacket *p = (luapacket*)lua_newuserdata(L, sizeof(*p));
		luaL_getmetatable(L, LUARAWPACKET_METATABLE);
		lua_setmetatable(L, -2);				
		p->_packet = (packet*)rawpacket_new(len);
		rawpacket_append((rawpacket*)p->_packet,data,len);
		return 1;
	}else if(argtype ==  LUA_TUSERDATA){
		luapacket *other = lua_topacket(L,1);
		if(!other)
			return luaL_error(L,"invaild opration for arg1");
		if(other->_packet->type != RAWPACKET)
			return luaL_error(L,"invaild opration for arg1");
		luapacket *p = (luapacket*)lua_newuserdata(L, sizeof(*p));
		luaL_getmetatable(L, LUARAWPACKET_METATABLE);
		lua_setmetatable(L, -2);
		p->_packet = clone_packet(other->_packet);
		return 1;							
	}else
		return luaL_error(L,"invaild opration for arg1");	
}

static int32_t 
lua_clone_packet(lua_State *L)
{
	luapacket *p,*other;
	if(lua_type(L,1) !=  LUA_TUSERDATA)
		return luaL_error(L,"arg1 should be a packet");
	other = lua_topacket(L,1);
	if(other->_packet->type == RPACKET){
		p = (luapacket*)lua_newuserdata(L, sizeof(*p));
		luaL_getmetatable(L, LUARPACKET_METATABLE);
	}else if(other->_packet->type == WPACKET){
		p = (luapacket*)lua_newuserdata(L, sizeof(*p));
		luaL_getmetatable(L, LUAWPACKET_METATABLE);
	}else if(other->_packet->type == RAWPACKET){
		p = (luapacket*)lua_newuserdata(L, sizeof(*p));
		luaL_getmetatable(L, LUARAWPACKET_METATABLE);		
	}else{
		return luaL_error(L,"invaild packet type");
	}
	lua_setmetatable(L, -2);	
	p->_packet = clone_packet(other->_packet);
	return 1;	
}

#define SET_FUNCTION(L,NAME,FUNC) do{\
	lua_pushstring(L,NAME);\
	lua_pushcfunction(L,FUNC);\
	lua_settable(L, -3);\
}while(0)

void 
reg_luapacket(lua_State *L)
{
    luaL_Reg rpacket_mt[] = {
        {"__gc", luapacket_gc},
        {NULL, NULL}
    };

    luaL_Reg wpacket_mt[] = {
        {"__gc", luapacket_gc},
        {NULL, NULL}
    };

    luaL_Reg rawpacket_mt[] = {
        {"__gc", luapacket_gc},
        {NULL, NULL}
    };        

    luaL_Reg rpacket_methods[] = {
        {"ReadU8",  _read_uint8},
        {"ReadU16", _read_uint16},
        {"ReadU32", _read_uint32},
        {"ReadU8",  _read_int8},
        {"ReadU16", _read_int16},
        {"ReadU32", _read_int32},        
        {"ReadNum", _read_double},        
        {"ReadStr", _read_string},
        {"ReadTab", _read_table},
        {"ReadBin", _read_rawbin},
        {NULL, NULL}
    };

    luaL_Reg wpacket_methods[] = {                 
        {"WriteU8", _write_uint8},
        {"WriteU16",_write_uint16},
        {"WriteU32",_write_uint32},
        {"WriteNum",_write_double},        
        {"WriteStr",_write_string},
        {"WriteTab",_write_table},
        {NULL, NULL}
    }; 

    luaL_Reg rawpacket_methods[] = {                 
		{"ReadBin", _read_rawbin},
        {NULL, NULL}
    };             

    luaL_newmetatable(L, LUARPACKET_METATABLE);
    luaL_setfuncs(L, rpacket_mt, 0);

    luaL_newlib(L, rpacket_methods);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    luaL_newmetatable(L, LUAWPACKET_METATABLE);
    luaL_setfuncs(L, wpacket_mt, 0);

    luaL_newlib(L, wpacket_methods);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    luaL_newmetatable(L, LUARAWPACKET_METATABLE);
    luaL_setfuncs(L, rawpacket_mt, 0);

    luaL_newlib(L, rawpacket_methods);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);



   	lua_newtable(L);

   	SET_FUNCTION(L,"rpacket",lua_new_rpacket);
   	SET_FUNCTION(L,"wpacket",lua_new_wpacket);
   	SET_FUNCTION(L,"rawpacket",lua_new_rawpacket);
   	SET_FUNCTION(L,"clone",lua_clone_packet);


}
