CFLAGS = -g -Wall -fno-strict-aliasing 
LDFLAGS = -lpthread -lrt -lm -lssl -lcrypto
SHARED  = -fPIC
DEPDIR  = 
INCLUDE = -I./ -I../deps/lua-5.3.0/src
DEFINE  =
LIBNAME = libchuck.a

# Platform-specific overrides
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
ifeq ($(uname_S),Linux)
	LDFLAGS += -ldl
	DEFINE += -D_LINUX
endif

ifeq ($(uname_S),FreeBSD)
	LDFLAGS += -lexecinfo
	DEFINE += -D_BSD
endif


source =  exception.c\
		  util/time.c\
		  util/minheap.c\
		  thread/thread.c\
		  thread/spinlock.c\
		  lua/lua_util.c\
		  log.c\
		  str.c\
		  mem/obj_allocator.c\
		  refobj.c

test-source = test/testexception.c\
			  test/testthread.c\

release:$(source)
	$(CC) $(SHARED) -O2 $(CFLAGS) -c $(source) $(INCLUDE) $(DEFINE)
	ar -rc $(LIBNAME) *.o

debug:$(source)
	$(CC) $(SHARED) $(CFLAGS) -c $(source) $(INCLUDE) $(DEFINE) -D_DEBUG
	ar -rc $(LIBNAME) *.o

test:$(test-source)
	$(CC) $(CFLAGS) -c $(test-source) $(INCLUDE) $(DEFINE)
	$(CC) $(CFLAGS) -o testexception testexception.o $(LIBNAME) $(INCLUDE) $(LDFLAGS) $(DEFINE)
	$(CC) $(CFLAGS) -o testthread testthread.o $(LIBNAME) $(INCLUDE) $(LDFLAGS) $(DEFINE)

clean:
	rm *.o
	rm *.a	