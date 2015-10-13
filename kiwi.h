#define KIWI_VERSION      "20141225"

#define KIWI_SERVER_NAME  "Kiwi/0.01"

#define KIWI_JSON_KEY_KIWI     "kiwi"
#define KIWI_JSON_KEY_VERSION  "version"
#define KIWI_JSON_KEY_POINT    "point"
#define KIWI_JSON_KEY_VALUE    "value"
#define KIWI_JSON_KEY_TIME     "time"

#define KIWI_TIME_MAXLEN	30    /* i.e. YYYY-mm-ddTHH:MM:SS.zzz+zzzz\0 */
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

struct tini_base;

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
	int (*db_get_limit)(struct kiwi_ctx *, const char *, const int, struct kiwi_chunk_key **);
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
    int, int, int, int (*)(struct sio_ctx *), int);
void kiwi_ev_loop(struct kiwi_ctx *);

struct kiwi_ctx {
	struct kiwi_chunk_key *head;

	struct kiwi_keymap **keymap;
	int keymap_len;

	int debug;
	int f_chroot;

	char *server_addr;
	char *server_port;

	struct tini_base *config;

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

	int cp_interval;
	int cp_current;
	int cp_pm_type;
};

extern int kiwi_debug;
extern struct kiwi_ctx *kiwi;

/* config */
void kiwi_config_print(struct kiwi_ctx *);
int kiwi_config_check_section(struct kiwi_ctx *, const char *);
const char *kiwi_config_get_v(struct kiwi_ctx *, const char *, const char *);
int kiwi_config_set_keymap(struct kiwi_ctx *);
void kiwi_config_load(struct kiwi_ctx *, const char *);
void kiwi_config_reload(struct kiwi_ctx *, const char *);

/* submit */
int kiwi_submit_file(char *);
int kiwi_submit_peer(struct kiwi_ctx *, struct kiwi_chunk_key *);
int kiwi_submit(struct kiwi_ctx *, struct kiwi_chunk_key *);

/* local db */
int kiwi_db_insert(struct kiwi_ctx *, struct kiwi_chunk_key *);
int kiwi_db_purge(struct kiwi_ctx *, char *, int);
int kiwi_db_get_latest(struct kiwi_ctx *, const char *, struct kiwi_xbuf *, struct kiwi_xbuf *);
int kiwi_db_get_limit(struct kiwi_ctx *, const char *, const int, struct kiwi_chunk_key **);
int kiwi_db_close(struct kiwi_ctx *);
int kiwi_set_db(struct kiwi_ctx *, int, char *, int);

//#include <microhttpd.h>
#ifdef USE_KIWI_SERVER
int kiwi_server_loop(struct kiwi_ctx *);
#endif

void kiwi_io_loop(struct kiwi_ctx *);

/* chunk */
#include <time.h>
char *kiwi_get_strtime(char *, int, long);
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
void kiwi_version(struct kiwi_ctx *);
int kiwi_set_debug(struct kiwi_ctx *, int);
void kiwi_dump(char *, int);
char *kiwi_find_keymap_hash(struct kiwi_ctx *, const char *);
int kiwi_set_keymap_tab(struct kiwi_ctx *, struct kiwi_keymap *, int);
int kiwi_set_server_param(struct kiwi_ctx *, char *, char *);
int kiwi_set_codec(struct kiwi_ctx *, int);
int kiwi_set_client_param(struct kiwi_ctx *, int, char *);
struct kiwi_ctx *kiwi_init(void);

/* kiwi_mp */
#define KIWI_CLIENT_MP_MIN   1
#define KIWI_CLIENT_MP_MAX   2
#define KIWI_CLIENT_MP_AVR   3
#define KIWI_CLIENT_MP_V3    4
int kiwi_mp_get_mean(struct kiwi_chunk_value *, char *, int);
int kiwi_mp_get_max(struct kiwi_chunk_value *, char *, int);
int kiwi_mp_get_min(struct kiwi_chunk_value *, char *, int);

/* xbuf */
void kiwi_xbuf_free(struct kiwi_xbuf *);
struct kiwi_xbuf *kiwi_xbuf_new(int);
size_t kiwi_xbuf_strcat(struct kiwi_xbuf *, const char *);
size_t kiwi_xbuf_vstrcat(struct kiwi_xbuf *, const char *, ...);
size_t kiwi_xbuf_memcpy(struct kiwi_xbuf *, size_t, const char *, size_t);
