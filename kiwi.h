#define KIWI_VERSION      "20141225"

#define KIWI_JSON_KEY_KIWI     "kiwi"
#define KIWI_JSON_KEY_VERSION  "version"
#define KIWI_JSON_KEY_POINT    "point"
#define KIWI_JSON_KEY_VALUE    "value"
#define KIWI_JSON_KEY_TIME     "time"

#define KIWI_TIME_MAXLEN	25	/* i.e. YYYY-mm-ddTHH:MM:SS+zzzz\0 */
#define KIWI_VALUE_MAXLEN	128

#define KIWI_CODEC_IEEE1888	1
#define KIWI_CODEC_JSON		2

#define KIWI_TRANSPORT_HTTP	1

#define KIWI_SQL_STRLEN 1024

#include "depends.h"

struct kiwi_xbuf {
	char *buf;
	int buflen;	/* maximum size of the buffer */
	int offset;	/* data length in the buffer */
};

struct kiwi_chunk_value {
	char *time;
	char *value;
	struct kiwi_chunk_value *next;
};

struct kiwi_chunk_key {
	char *key;
	struct kiwi_chunk_value *value;
	struct kiwi_chunk_key *next;
};

struct kiwi_keymap {
        char *key;
        char *hash;
};
#define KIWI_KEYMAP_SIZE(d) sizeof(d)/sizeof(d[0])

struct kiwi_cf_base;

/*
 * db 
 */
struct kiwi_ctx;
struct kiwi_db_ctx {
	int db_type;
#define KIWI_DBTYPE_RINGBUF	1
#define KIWI_DBTYPE_SQLITE3	2
#define KIWI_DBTYPE_MONGODB	3
	void *db;	/* context pointer to the actual database. */
	char *db_name;
	int db_max_size;
	int (*db_open)(struct kiwi_ctx *);
	int (*db_close)(struct kiwi_ctx *);
	int (*db_insert)(struct kiwi_ctx *, struct kiwi_chunk_key *);
	int (*db_purge)(struct kiwi_ctx *, char *, int);
	int (*db_get_latest)(struct kiwi_ctx *, const char *, struct kiwi_xbuf *, struct kiwi_xbuf *);
};

#include "ringbuf/ringbuf.h"
int kiwi_ringbuf_init(struct kiwi_ctx *);

int ieee1888_encode(struct kiwi_ctx *, struct kiwi_chunk_key *, struct kiwi_xbuf *);
int ieee1888_transmit(struct kiwi_ctx *, struct kiwi_xbuf *);

#ifdef USE_KIWI_TRANSPORT_XMPP
#endif

#ifdef USE_KIWI_TRANSPORT_MQTT
#endif

#include "simple_event/simple_event.h"
#include "simple_sio/simple_sio.h"

void kiwi_ev_init(struct kiwi_ctx *);
int kiwi_set_event_sio(struct kiwi_ctx *, char *, int,
    int, int, int (*)(struct sio_ctx *), int);
void kiwi_ev_loop(struct kiwi_ctx *);

struct kiwi_ctx {
	struct kiwi_chunk_key *head;

	struct kiwi_keymap *keymap;
	int keymap_len;

	/* XXX move to cf_base */
	int debug;
	int f_chroot;

//#ifdef USE_KIWI_SERVER
	char *server_addr;
	char *server_port;
//#endif

	struct kiwi_cf_base **cf;
	int cf_num;

	int codec;
	int (*encode)(struct kiwi_ctx *, struct kiwi_chunk_key *, struct kiwi_xbuf *);

	int transport;
	int (*transmit)(struct kiwi_ctx *, struct kiwi_xbuf *);
	char *peer_name;

	struct simple_ev_ctx *ev_ctx;

	/*
	 * TODO
	 * status container for any DB
	 *   i.e.
	 *     sqlite3: sqlite3 *
	 *     mongodb: mongoc_client_t *
	 *
	 */
	struct kiwi_db_ctx *db_ctx;
};

extern int kiwi_debug;
extern struct kiwi_ctx *kiwi;

//#ifdef USE_KIWI_CONFIG
/* config */
void kiwi_config_dump(struct kiwi_ctx *);
void kiwi_config_load(struct kiwi_ctx *, char *);
void kiwi_config_reload(struct kiwi_ctx *, char *);
//#endif

//#ifdef USE_KIWI_SUBMIT
/* submit */
int kiwi_submit_file(char *);
int kiwi_submit(struct kiwi_ctx *, struct kiwi_chunk_key *);
//#endif

//#ifdef USE_KIWI_DB
/* local db */
int kiwi_db_insert(struct kiwi_ctx *, struct kiwi_chunk_key *);
int kiwi_db_purge(struct kiwi_ctx *, char *, int);
int kiwi_db_get_latest(struct kiwi_ctx *, const char *, struct kiwi_xbuf *, struct kiwi_xbuf *);
int kiwi_db_close(struct kiwi_ctx *);
int kiwi_set_db(struct kiwi_ctx *, int, char *, int);
//#endif

//#ifdef USE_KIWI_SERVER
//#include <microhttpd.h>
int kiwi_server_loop(struct kiwi_ctx *);
//#endif

//#ifdef USE_KIWI_IO
void kiwi_io_loop(struct kiwi_ctx *);
//#endif

/* chunk */
#include <time.h>
char *kiwi_get_time(char *, int, time_t);
char *kiwi_time_canon(char *, char *, int);
void kiwi_chunk_dump_value(struct kiwi_chunk_value *);
void kiwi_chunk_dump_key_list(struct kiwi_chunk_key *);
void kiwi_chunk_dump(struct kiwi_chunk_key *);
void kiwi_chunk_free_value(struct kiwi_chunk_value *);
void kiwi_chunk_free(struct kiwi_chunk_key *);
struct kiwi_chunk_value *kiwi_chunk_add_value(struct kiwi_chunk_value **, char *, char *);
struct kiwi_chunk_value *kiwi_chunk_key_add_value(struct kiwi_chunk_key *, char *, char *);
struct kiwi_chunk_key *kiwi_chunk_add_key(struct kiwi_chunk_key **, char *);
struct kiwi_chunk_key *kiwi_chunk_add(struct kiwi_chunk_key **, char *, char *, char *);

/* kiwi.c */
int kiwi_set_debug(struct kiwi_ctx *, int);
void kiwi_dump(char *, int);
char *kiwi_find_keymap_hash(struct kiwi_ctx *, const char *);
int kiwi_set_keymap(struct kiwi_ctx *, struct kiwi_keymap *, int);
int kiwi_set_server_param(struct kiwi_ctx *, char *, char *);
int kiwi_set_codec(struct kiwi_ctx *, int);
int kiwi_set_client_param(struct kiwi_ctx *, int, char *);
struct kiwi_ctx *kiwi_init(void);

/* xbuf */
void kiwi_xbuf_free(struct kiwi_xbuf *);
struct kiwi_xbuf *kiwi_xbuf_new(int);
size_t kiwi_xbuf_strcat(struct kiwi_xbuf *, const char *);
size_t kiwi_xbuf_vstrcat(struct kiwi_xbuf *, const char *, ...);
size_t kiwi_xbuf_memcpy(struct kiwi_xbuf *, size_t, const char *, size_t);
