/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */

#ifndef _LIBMCTP_H
#define _LIBMCTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef uint8_t mctp_eid_t;

/* MCTP packet definitions */
struct mctp_hdr {
	uint8_t	ver;
	uint8_t	dest;
	uint8_t	src;
	uint8_t	flags_seq_tag;
};

/* Definitions for flags_seq_tag field */
#define MCTP_HDR_FLAG_SOM	(1<<7)
#define MCTP_HDR_FLAG_EOM	(1<<6)
#define MCTP_HDR_FLAG_TO	(1<<3)
#define MCTP_HDR_SEQ_SHIFT	(4)
#define MCTP_HDR_SEQ_MASK	(0x3)
#define MCTP_HDR_TAG_SHIFT	(0)
#define MCTP_HDR_TAG_MASK	(0x7)

/* Maximum size of *payload* data in a MCTP packet
 * @todo: dynamic sixing based on channel implementation.
 */
#define MCTP_MTU	248

/* packet buffers */

/* Allow a little space before the MCTP header in the packet, for bindings that
 * may add their own header
 */
#define MCTP_PKTBUF_BINDING_PAD	2

#define MCTP_PKTBUF_SIZE	(MCTP_PKTBUF_BINDING_PAD + \
		(sizeof(struct mctp_hdr) + MCTP_MTU))

struct mctp_pktbuf {
	unsigned char	data[MCTP_PKTBUF_SIZE];
	uint8_t		start, end;
	uint8_t		mctp_hdr_off;
	struct mctp_pktbuf *next;
};

struct mctp_pktbuf *mctp_pktbuf_alloc(uint8_t len);
void mctp_pktbuf_free(struct mctp_pktbuf *pkt);
struct mctp_hdr *mctp_pktbuf_hdr(struct mctp_pktbuf *pkt);
void *mctp_pktbuf_data(struct mctp_pktbuf *pkt);
uint8_t mctp_pktbuf_size(struct mctp_pktbuf *pkt);
void *mctp_pktbuf_alloc_start(struct mctp_pktbuf *pkt, uint8_t size);
void *mctp_pktbuf_alloc_end(struct mctp_pktbuf *pkt, uint8_t size);
int mctp_pktbuf_push(struct mctp_pktbuf *pkt, void *data, uint8_t len);

/* MCTP core */
struct mctp;
struct mctp_bus;
struct mctp_binding;

struct mctp *mctp_init(void);

/* Register a binding to the MCTP core, and creates a bus (populating
 * binding->bus).
 */
int mctp_register_bus(struct mctp *mctp,
		struct mctp_binding *binding,
		mctp_eid_t eid);

typedef void (*mctp_rx_fn)(uint8_t src_eid, void *data,
		void *msg, size_t len);

int mctp_set_rx_all(struct mctp *mctp, mctp_rx_fn fn, void *data);

int mctp_message_tx(struct mctp *mctp, mctp_eid_t eid,
		void *msg, size_t msg_len);

/* hardware bindings */
struct mctp_binding {
	const char	*name;
	uint8_t		version;
	struct mctp_bus	*bus;
	struct mctp	*mctp;
	int		(*tx)(struct mctp_binding *binding,
				struct mctp_pktbuf *pkt);
};

void mctp_binding_set_tx_enabled(struct mctp_binding *binding, bool enable);

void mctp_bus_rx(struct mctp_binding *binding, struct mctp_pktbuf *pkt);

/* environment-specific allocation */
void mctp_set_alloc_ops(void *(*alloc)(size_t),
		void (*free)(void *),
		void *(realloc)(void *, size_t));

/* environment-specific logging */

void mctp_set_log_stdio(int level);
void mctp_set_log_syslog(void);
void mctp_set_log_custom(void (*fn)(int, const char *, va_list));

/* these should match the syslog-standard LOG_* definitions, for
 * easier use with syslog */
#define MCTP_LOG_ERR		3
#define MCTP_LOG_WARNING	4
#define MCTP_LOG_NOTICE		5
#define MCTP_LOG_INFO		6
#define MCTP_LOG_DEBUG		7


#ifdef __cplusplus
}
#endif

#endif /* _LIBMCTP_H */
