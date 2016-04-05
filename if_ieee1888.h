#ifndef IF_IEEE1888_H_
#define IF_IEEE1888_H_

int if_ieee1888_init(struct kiwi_ctx *);
int if_ieee1888_json_encode(struct kiwi_ctx *, struct kiwi_chunk_key *, struct kiwi_xbuf *);
int if_ieee1888_soap_encode(struct kiwi_ctx *, struct kiwi_chunk_key *, struct kiwi_xbuf *);
int if_ieee1888_http_transmit(struct kiwi_ctx *, struct kiwi_xbuf *);

#endif /* IF_IEEE1888_H_ */
