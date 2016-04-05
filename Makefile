include Makefile.defs

BASENAME = kiwi

ifdef SHARED
TARGETS = lib$(BASENAME).so
else
TARGETS = lib$(BASENAME).a
endif

all: $(TARGETS)

OBJS += kiwi.o kiwi_time.o kiwi_chunk.o xbuf.o
OBJS += submit.o kiwi_mp.o

OBJS += kiwi_ev.o simple_event/simple_event.o
OBJS += simple_sio/simple_sio.o

# base
CFLAGS += -Wall -Werror
ifdef SHARED
CFLAGS += -fPIC
LDFLAGS += -shared
#LDFLAGS += -Wl,-soname,lib$(BASENAME).so
#CFLAGS += -Wextra
endif
CFLAGS += -g -O
CPPFLAGS += -I$(KIWI_BASE_DIR)
LDFLAGS += -L$(KIWI_BASE_DIR)

ifndef NO_USE_KIWI_SERVER
USE_KIWI_SERVER=1
endif
ifndef NO_USE_KIWI_CLIENT
USE_KIWI_CLIENT=1
endif

ifdef KIWI_TIME_WITH_MS
CPPFLAGS += -DKIWI_TIME_WITH_MS
endif

# library for data base
OBJS += db.o
OBJS += kiwi_ringbuf.o
OBJS += ringbuf/ringbuf.o

# sqlite3
ifdef USE_KIWI_DB_SQLITE3
CPPFLAGS += -DUSE_KIWI_DB_SQLITE3
OBJS += if_sqlite3.o
LDLIBS += -lsqlite3
if_sqlite3.o: if_sqlite3.h
endif

# library for client.
#   default codec is IEEE1888.
#   default client is libcurl.
ifdef USE_KIWI_CLIENT
CPPFLAGS += -DUSE_KIWI_CLIENT
USE_KIWI_TRANSPORT_IEEE1888_SOAP=1
CPPFLAGS += -DUSE_KIWI_CLIENT_CURL
# libcurl
OBJS += if_curl.o
LDLIBS += -lcurl
if_curl.o: if_curl.h
endif

# library for server
ifdef USE_KIWI_SERVER
CPPFLAGS += -DUSE_KIWI_SERVER
# libmicrohttpd
ifdef USE_KIWI_SERVER_MHD
CPPFLAGS += -DUSE_KIWI_SERVER_MHD
ifdef MHD_INC_DIR
CPPFLAGS += -I$(MHD_INC_DIR)
endif
ifdef MHD_LIB_DIR
LDFLAGS += -L$(MHD_LIB_DIR)
endif
OBJS += if_mhd_select.o
LDLIBS += -lmicrohttpd
endif
endif

# library for transport
ifdef USE_KIWI_TRANSPORT_IEEE1888_SOAP
CPPFLAGS += -DUSE_KIWI_TRANSPORT_IEEE1888_SOAP
OBJS += if_ieee1888.o
if_ieee1888.o: if_ieee1888.h
endif
ifdef USE_KIWI_TRANSPORT_JSON
CPPFLAGS += -DUSE_KIWI_TRANSPORT_JSON
LDLIBS += -ljansson
endif
ifdef USE_KIWI_TRANSPORT_KII_HTTP
CPPFLAGS += -DUSE_KIWI_TRANSPORT_KII_HTTP
OBJS += if_kii.o
if_kii.o: if_kii.h
endif

# library for configuration
OBJS += kiwi_config.o tini/tini.o
CPPFLAGS += -DUSE_KIWI_CONFIG

lib$(BASENAME).a: $(OBJS)
	ar rv $@ $^

lib$(BASENAME).so: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(SONAME).0.1: $(OBJS)

$(OBJS): kiwi.h

clean::
	rm -rf $(OBJS) $(TARGETS) *.dSYM *.o

.PHONY: clean

