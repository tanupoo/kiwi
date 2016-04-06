kiwi
====

** DON'T TRUST THIS DOCUMENT. IT SHOULD BE RE-WRITTEN SOON **

kiwi is a library to make a field/edge/fog gateway.

## Requirements

libcurl 7.48.0
libmicrohttpd 0.9.48

if you need sqlite as your storage,
    sqlite3
    sqlite3-dev

libjansson

## tips

### compiling of libcurl 7.48.0

options for the configure

    ~~~~
    ./configure \
        --enable-debug \
        --enable-ipv6 \
        --disable-ftp \
        --disable-file \
        --disable-ldap  \
        --disable-ldaps \
        --disable-rtsp \
        --disable-dict \
        --disable-telnet \
        --disable-tftp \
        --disable-pop3 \
        --disable-imap \
        --disable-smb \
        --disable-smtp \
        --disable-gopher \
        --disable-manual \
        --disable-sspi \
        --disable-ntlm-wb \
        --disable-tls-srp \
        --without-winssl \
        --without-darwinssl \
        --without-gnutls \
        --without-polarssl \
        --without-mbedtls \
        --without-cyassl \
        --without-nss \
        --without-axtls \
        --without-libpsl \
        --without-libmetalink \
        --without-libssh2 \
        --without-librtmp \
        --without-winidn \
        --without-libidn \
        --without-nghttp2 \
        --without-zsh-functions-dir \
        --prefix=${HOME}/apps/curl
    ~~~~

result of the configure.

    ~~~~
    configure: Configured to build curl/libcurl:
    
      curl version:     7.48.0
      Host setup:       i686-pc-linux-gnu
      Install prefix:   /home/sakane/apps/curl
      Compiler:         gcc
      SSL support:      enabled (OpenSSL)
      SSH support:      no      (--with-libssh2)
      zlib support:     enabled
      GSS-API support:  no      (--with-gssapi)
      TLS-SRP support:  no      (--enable-tls-srp)
      resolver:         default (--enable-ares / --enable-threaded-resolver)
      IPv6 support:     enabled
      Unix sockets support: enabled
      IDN support:      no      (--with-{libidn,winidn})
      Build libcurl:    Shared=yes, Static=yes
      Built-in manual:  no      (--enable-manual)
      --libcurl option: enabled (--disable-libcurl-option)
      Verbose errors:   enabled (--disable-verbose)
      SSPI support:     no      (--enable-sspi)
      ca cert bundle:   /etc/ssl/certs/ca-certificates.crt
      ca cert path:     no
      ca fallback:      no
      LDAP support:     no      (--enable-ldap / --with-ldap-lib / --with-lber-lib)
      LDAPS support:    no      (--enable-ldaps)
      RTSP support:     no      (--enable-rtsp)
      RTMP support:     no      (--with-librtmp)
      metalink support: no      (--with-libmetalink)
      PSL support:      no      (--with-libpsl)
      HTTP2 support:    disabled (--with-nghttp2)
      Protocols:        HTTP HTTPS
    ~~~~

### compiling of libmicrohttpd 0.9.48

options for the configure

    ~~~~
    CFLAGS=-g ./configure \
        --disable-curl \
        --enable-https \
        --prefix=$HOME/apps/mhd
    ~~~~

result of the configure.

    ~~~~
    configure: libmicrohttpd 0.9.48 Configuration Summary:
      Cross-compiling:   no
      Operating System:  linux-gnu
      Threading lib:     posix
      libcurl (testing): no, many unit tests will not run
      Target directory:  /home/sakane/apps/mhd
      Messages:          yes
      Basic auth.:       yes
      Digest auth.:      yes
      Postproc:          yes
      HTTPS support:     yes (using libgnutls and libgcrypt)
      poll support:      yes
      epoll support:     yes
      build docs:        yes
      build examples:    yes
    
    configure: HTTPS subsystem configuration:
      License         :  LGPL only
    ~~~~

## mac

sqlite3

## wren

jquery
jquery.flot

##

USE_KIWI

USE_KIWI_CLIENT
    default transport protocol is http.
        default client is libcurl.
    default codec is ieee1888.

USE_KIWI_SERVER
    default transport protocol is http.
        default server is libmhd.
    default codec is ieee1888.

USE_KIWI_CONFIG

### database

default is a ring buffer.
USE_KIWI_DB_SQLITE3
USE_KIWI_DB_MONGODB

### transport protocol

default is IEEE1888+SOAP.

USE_KIWI_TRANSPORT_IEEE1888_SOAP
USE_KIWI_TRANSPORT_KII_HTTP
USE_KIWI_TRANSPORT_XMPP_JSON (not yet)
USE_KIWI_TRANSPORT_MQTT_JSON (not yet)

    ~~~~
    *Application*
    |
    (libkiwi)
    makes a chunk.
    stores data to a local DB
        to memory
        to sqlite
    sends data to the peer  ----> *storage*
        by ieee1888
            using curl
    ~~~~

    ~~~~
    *Application*
    |
    (libkiwi)
    setopt
        with_config
        cache_config
        with_server
    makes a chunk.
    |
    v
    submits the chunk.
    |
    |                          <-- HTTP GET ---
    (with server?) ---> *kiwi* --- response --> *client*
    |                     |
    |                     |
    v                     v
    stores data to a local DB
        to memory
        to sqlite
    sends data to the peer  ----> *storage*
        by ieee1888
            by curl
    ~~~~

## configuration

    [keymap]

    [section]
    key = value

    kiwi_config_load(kiwi, config_file);
    kiwi_config_check_section(kiwi, section_name);
    kiwi_config_get_v(kiwi, key);
        

## configuration example

### kiwi_setopt()

## http server

    ~~~~
    ?key=<keystring>&<keystring>

    keystring :=
        e.g. ?key=http://fiap.tanu.org/test/alps0xf1/temperature
        it must not contain either [?&=].
    ~~~~

###

    [default]
        mode = quick
        compress = false
        encoding = xml
        transport = http

    [sensor_data_001]
        mode = batch
        unit = 300

###

    [default]
        mode = batch
        unit = 300
        compress = false
        encoding = xml
        transport = http

    [http://fiap.tanu.org/test/id001]
        mode = quick

## request format

    ~~~~
    <json> := {
      "kiwi" : {
        "version" : "20141225",
        "point" : { <point spec>, ... }
      }
    
    <point spec> := 
      "<point id>" : [ <value spec>, ... ] 

    <value spec> := {
      "time" : <time spec>,
      "value" : <value> }
    ~~~~

### example

- point query

    ~~~~
    {
      "kiwi": {
        "version": "20141225",
        "point": {
          "http://fiap.tanu.org/test/alps01/temp": [
            { "time": "2014-11-21T07:54:03+0900", "value": "26.0" },
            { "time": "2014-11-21T07:55:00+0900", "value": "26.5" }
          ],
          "http://fiap.tanu.org/test/alps01/light": [
            { "time": "2014-11-21T07:54:03+0900", "value": "1301" },
            { "time": "2014-11-21T07:55:00+0900", "value": "1400" }
          ]
        }
      }
    }
    ~~~~
