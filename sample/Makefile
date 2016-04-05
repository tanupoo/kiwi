include ../Makefile.defs

#NO_USE_KIWI_CLIENT=1
#NO_USE_KIWI_SERVER=1

ifndef NO_USE_KIWI_SERVER
USE_KIWI_SERVER=1
endif
ifndef NO_USE_KIWI_CLIENT
USE_KIWI_CLIENT=1
endif

TARGETS+= sin_quick_submit
#TARGETS+= sin_batch_submit
TARGETS+= trigon_quick

LDLIBS += -lm

# base
CPPFLAGS += -I$(KIWI_BASE_DIR)
LDFLAGS += -L$(KIWI_BASE_DIR)
LDLIBS += -lkiwi

# library for data base
ifdef USE_KIWI_DB_SQLITE3
CPPFLAGS += -DUSE_KIWI_DB_SQLITE3
endif
#LDFLAGS += -lsqlite3

ifdef USE_KIWI_CLIENT
CPPFLAGS += -DUSE_KIWI_CLIENT
USE_KIWI_TRANSPORT_IEEE1888_SOAP=1
CPPFLAGS += -DUSE_KIWI_CLIENT_CURL
endif
LDLIBS += -lcurl

# library for server
ifdef USE_KIWI_SERVER
CPPFLAGS += -DUSE_KIWI_SERVER
ifdef USE_KIWI_SERVER_MHD
CPPFLAGS += -DUSE_KIWI_SERVER_MHD
CPPFLAGS += -I$(MHD_INC_DIR)
endif
endif
LDFLAGS += -L$(MHD_LIB_DIR)
LDLIBS += -lmicrohttpd

# library for codec
ifdef USE_KIWI_TRANSPORT_IEEE1888_SOAP
CPPFLAGS += -DUSE_KIWI_TRANSPORT_IEEE1888_SOAP
endif
ifdef USE_KIWI_TRANSPORT_JSON
CPPFLAGS += -DUSE_KIWI_TRANSPORT_JSON
LDLIBS += -ljansson
endif

# library for configuration
CPPFLAGS += -DUSE_KIWI_CONFIG

ifeq ($(OS),Linux)
#LDFLAGS+= -lpthread
endif

all: $(TARGETS)

$(TARGETS):

#
#.SUFFIXES: .o .c
#
#.c.o:
#	$(CC) $(CFLAGS) -c $<

clean:
	-rm -rf a.out *.dSYM *.o
ifneq ($(TARGETS),)
	-rm -f $(TARGETS)
endif

distclean: clean
	-rm -f config.cache config.status config.log .depend

show-options:
	@echo CPPFLAGS =$(CPPFLAGS)
	@echo   CFLAGS =$(CFLAGS)
	@echo  LDFLAGS =$(LDFLAGS)
	@echo   LDLIBS =$(LDLIBS)

