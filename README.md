chunk.c: In function 'kiwi_time_canon':
chunk.c:75:2: warning: incompatible implicit declaration of built-in function
'snprintf' [enabled by default]
chunk.c: In function 'kiwi_chunk_add_value':
chunk.c:179:20: warning: incompatible implicit declaration of built-in function
'strdup' [enabled by default]
chunk.c: In function 'kiwi_chunk_add_key':
chunk.c:241:18: warning: incompatible implicit declaration of built-in function
'strdup' [enabled by default]

##

if you need sqlite as your storage,
    sqlite3
    sqlite3-dev

if you need http as your transport,
    libcurl
    libcurl4-openssl-dev

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

### codec

default is ieee1888.
USE_KIWI_CODEC_JSON

### transport protocol

USE_KIWI_TRANSPORT_XMPP
USE_KIWI_TRANSPORT_MQTT

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

    mode :=
        quick
        batch

    span :=
        <number in seconds>

    compress :=
        true
        false

    storage :=
        memory
        file
        sqlite

    encoding :=
        raw
        xml (ieee1888)
        json
        bpack

    transport :=
        file
        http (tcp)
        coap (udp)

    dest :=
        <URL in string>

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
