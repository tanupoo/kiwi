USE_KIWI_SERVER=1

TARGETS = kiwi libkiwi.a

TEST_TARGETS = test_xbuf test_chunk
TEST_TARGETS += test_config test_submit
TEST_TARGETS += test_ieee1888 test_http_curl test_mhd

CFLAGS = -g -Wall -Werror

include Makefile.defs

OBJS += kiwi.o chunk.o xbuf.o
OBJS += submit.o

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
USE_KIWI_CODEC_IEEE1888=1
#ifdef USE_KIWI_SERVER_MHD
CPPFLAGS += -DUSE_KIWI_SERVER_MHD
CPPFLAGS += -I$(MHD_INC_DIR)
LDFLAGS += -L$(MHD_LIB_DIR) -lmicrohttpd
OBJS += mhd_select.o
#endif
endif

# library for codec
ifdef USE_KIWI_CODEC_IEEE1888
CPPFLAGS += -DUSE_KIWI_CODEC_IEEE1888
OBJS += ieee1888.o
endif
ifdef USE_CODEC_JSON
CPPFLAGS += -DUSE_KIWI_CODEC_JSON
LDFLAGS += -ljansson
endif

# library for configuration
ifdef USE_KIWI_CONFIG
OBJS += kiwi_config.o inih/ini.o
CPPFLAGS += -DUSE_KIWI_CONFIG
endif

#kiwi:

libkiwi.a: $(OBJS)
	ar rv $@ $^

test: $(TEST_TARGETS)

test_xbuf: test_xbuf.o xbuf.o

test_chunk: test_chunk.o chunk.o

test_config: test_config.o

test_db: test_db.o

test_submit: test_submit.o

test_mhd: test_mhd.o

test_http_curl: http_curl.o

test_ieee1888: http_curl.o chunk.o ieee1888.o

clean::
	rm -rf $(OBJS) $(TARGETS) $(TEST_TARGETS) *.dSYM *.o

.PHONY: test clean

