ifndef NO_USE_KIWI_SERVER
USE_KIWI_SERVER=1
endif
ifndef NO_USE_KIWI_CLIENT
USE_KIWI_CLIENT=1
endif

TARGETS = kiwi libkiwi.a

TEST_TARGETS = test_xbuf test_chunk
TEST_TARGETS += test_config test_submit

CFLAGS = -g -Wall -Werror

include Makefile.defs

OBJS += kiwi.o kiwi_time.o kiwi_chunk.o xbuf.o
OBJS += submit.o kiwi_mp.o

OBJS += kiwi_ev.o simple_event/simple_event.o
OBJS += simple_sio/simple_sio.o

# library for data base
OBJS += db.o
OBJS += kiwi_ringbuf.o
OBJS += ringbuf/ringbuf.o
ifdef USE_KIWI_DB_SQLITE3
OBJS += kiwi_sqlite3.o
CPPFLAGS += -DUSE_KIWI_DB_SQLITE3
LDFLAGS += -lsqlite3
endif

# library for client.
#   default codec is IEEE1888.
#   default client is libcurl.
ifdef USE_KIWI_CLIENT
CPPFLAGS += -DUSE_KIWI_CLIENT
USE_KIWI_CODEC_IEEE1888=1
OBJS += http_curl.o
CPPFLAGS += -DUSE_KIWI_CLIENT_CURL
LDFLAGS += -lcurl
endif

# library for server
#   default codec is IEEE1888.
#   default server is libmhd.
ifdef USE_KIWI_SERVER
CPPFLAGS += -DUSE_KIWI_SERVER
USE_KIWI_SERVER_MHD=1
USE_KIWI_CODEC_IEEE1888=1
CPPFLAGS += -DUSE_KIWI_SERVER_MHD
CPPFLAGS += -I$(MHD_INC_DIR)
LDFLAGS += -L$(MHD_LIB_DIR) -lmicrohttpd
OBJS += mhd_select.o
endif

# library for codec
ifdef USE_KIWI_CODEC_IEEE1888
CPPFLAGS += -DUSE_KIWI_CODEC_IEEE1888
OBJS += ieee1888.o
endif
ifdef USE_KIWI_CODEC_JSON
CPPFLAGS += -DUSE_KIWI_CODEC_JSON
LDFLAGS += -ljansson
endif

# library for configuration
OBJS += kiwi_config.o tini/tini.o
CPPFLAGS += -DUSE_KIWI_CONFIG

#kiwi:

libkiwi.a: $(OBJS)
	ar rv $@ $^

test: $(TEST_TARGETS)

test_xbuf: test_xbuf.o xbuf.o

test_chunk: test_chunk.o kiwi_time.o kiwi_chunk.o

test_config: test_config.o kiwi_config.o tini/tini.o

test_sqlite3: test_sqlite3.o

test_submit: test_submit.o kiwi_time.o

test_mhd: test_mhd.o

test_http_curl: http_curl.o

test_ieee1888: http_curl.o kiwi_time.o kiwi_chunk.o ieee1888.o

clean::
	rm -rf $(OBJS) $(TARGETS) $(TEST_TARGETS) *.dSYM *.o

.PHONY: test clean

