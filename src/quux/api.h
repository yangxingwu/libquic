#ifndef QUUX_API_H_
#define QUUX_API_H_

#ifdef __cplusplus
class quux_listener_s;
class quux_peer_s;
class quux_stream_s;
typedef class quux_listener_s* quux_listener;
typedef class quux_peer_s* quux_peer;
typedef class quux_stream_s* quux_stream;
extern "C" {

#else
typedef struct quux_listener_s* quux_listener;
typedef struct quux_peer_s* quux_peer;
typedef struct quux_stream_s* quux_stream;
#endif /* __cplusplus */

typedef void (*quux_acceptable)(quux_peer);
typedef void (*quux_cb)(quux_stream);

/*
 * Callbacks are triggered once when IO becomes actionable, at which point no callback will be triggered until
 * the read/write operation on that stream has returned 0.
 *
 * quux_c begins in triggered state for both read/write, which means IO can be attempted (albeit may return 0).
 */

/**
 * Initialise the module, specifying that the built in quux_loop will be used.
 */
int quux_init_loop(void);

struct event_base;

/**
 * Initialise the module, with your own libevent loop being used
 *
 * EVLOOP_ONCE *must* be used in the event_base_loop call,
 * otherwise quux's internal time cache will start to go stale.
 */
void quux_event_base_loop_init(struct event_base*);

void quux_set_peer_context(quux_peer, void* ctx);
void* quux_get_peer_context(quux_peer);

void quux_set_stream_context(quux_stream, void* ctx);
void* quux_get_stream_context(quux_stream);

/**
 * Start a listener for new streams on IPv4 addr.
 *
 * quux_acceptable cb is called with the relevant quux_conn and ctx when a fresh client connects.
 *
 * TODO: error if there is already a server listening on ip:port
 */
quux_listener quux_listen(const struct sockaddr* addr, quux_acceptable cb);

quux_stream quux_accept(quux_peer peer, quux_cb quux_writeable, quux_cb quux_readable);

/**
 * A handle representing an IPv4 connection to the peer
 */
quux_peer quux_open(const struct sockaddr* addr, quux_acceptable cb);

/**
 * Create a new stream on the connection conn.
 *
 * ctx will automatically be supplied to the callbacks when they activate
 */
quux_stream quux_connect(quux_peer peer, quux_cb quux_writeable, quux_cb quux_readable);

/**
 * Pass up to 'count' octets from 'buf' to the stream for send.
 *
 * Returned amount tells us how much data was transfered.
 * 0 indicates that no data could be written at this time, but the callback has been re-registered.
 * Call 'quux_write_is_closed' to find out if the stream is no longer writeable.
 *
 * The initial behaviour will be that once quux_read_close();quux_write_close(); have been called,
 * it's at the discretion of the impl to wait as long as necessary to receive acks for data before tearing down.
 * At some point more functions could be added to query the status of buffered data and force remove if needed.
 */
size_t quux_write(quux_stream stream, const uint8_t* buf, size_t count);

/**
 * Indicate we don't want to write any additional data to the stream.
 */
void quux_write_close(quux_stream stream);

/**
 * 1 if all future writes would do nothing, 0 otherwise.
 */
int quux_write_is_closed(quux_stream stream);

/**
 * Read up to 'count' octets from the stream into 'buf'
 *
 * Returned amount tells us how much data was transfered.
 * 0 indicates that no data could be read at this time, but the callback has been re-registered.
 * Call 'quux_read_is_closed' to find out if the stream is no longer readable.
 */
size_t quux_read(quux_stream stream, uint8_t* buf, size_t count);

/**
 * Indicate we don't want to read any additional data from the stream.
 */
void quux_read_close(quux_stream stream);

/**
 * 1 if all future reads would return nothing, 0 otherwise.
 */
int quux_read_is_closed(quux_stream stream);

/**
 * Stop accepting connections
 */
void quux_shutdown(quux_listener server);

/**
 * Run the built-in epoll event loop forever.
 *
 * The function will return after timeout_ms has elapsed,
 * or never if timeout_ms is set to -1
 */
void quux_loop(void);

/**
 * Run the built-in epoll event loop.
 *
 * The function will return after timeout_ms has elapsed.
 */
void quux_loop_with_timeout(int timeout_ms);

/**
 * Run this just after libevent wait wakes up.
 *
 * It updates the approximate time internal to QUIC.
 */
void quux_event_base_loop_before(void);

/**
 * Run this just before going into libevent wait.
 *
 * It sends any packets that were generated in the previous event loop run.
 */
void quux_event_base_loop_after(void);

#ifdef __cplusplus
}
#endif

#endif /* QUUX_API_H_ */
