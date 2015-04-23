CFLAGS = -g -Wall -fno-strict-aliasing 
LDFLAGS = -lpthread -lrt -lm
SHARED  = -fPIC
INCLUDE = -I../../ -I../../../deps/lua-5.3.0/src
DEFINE  =
LIB     = ../../../deps/lua-5.3.0/src/liblua.a

# Platform-specific overrides
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
ifeq ($(uname_S),Linux)
	LDFLAGS += -ldl
	DEFINE += -D_LINUX
endif

ifeq ($(uname_S),FreeBSD)
	DEFINE += -D_BSD
endif

all:
	$(CC) $(CFLAGS)  -o testlua testlua.c ../lua_util.c $(LIB) $(INCLUDE) $(LDFLAGS) $(DEFINE)