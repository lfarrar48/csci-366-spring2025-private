#ifndef MSU_SOCKET_H
#define MSU_SOCKET_H

#include <msulib/str.h>

#include <stddef.h>
#include <stdint.h>

typedef struct socketaddr socket_addr_t;
typedef struct tcplistener tcp_listener_t;
typedef struct tcpstream tcp_stream_t;

typedef enum tcperror {
    TCP_ERROR_NONE,
    TCP_ERROR_INIT,
    TCP_ERROR_ADDR_RESOLUTION,
    TCP_ERROR_INVALID_ADDR,
    TCP_ERROR_SOCKET_CREATION,
    TCP_ERROR_SOCKET_BIND,
    TCP_ERROR_SOCKET_LISTEN,
    TCP_ERROR_SOCKET_ACCEPT,
    TCP_ERROR_SOCKET_READ,
    TCP_ERROR_SOCKET_WRITE,
    TCP_ERROR_CLOSED,
} tcp_error_t;
const char *tcp_error_message(tcp_error_t err);

typedef struct tcpconn {
    tcp_stream_t *stream;
    socket_addr_t *peer_address;
} tcp_conn_t;

void sockets_init(tcp_error_t *error);
void sockets_cleanup();

socket_addr_t *socket_resolve_addr(const char *host, uint16_t port, tcp_error_t *errout);
tcp_listener_t *tcp_bind(socket_addr_t *addr, tcp_error_t *errout);
tcp_conn_t tcp_accept(tcp_listener_t *server, tcp_error_t *errout);

const msu_str_t *tcplistener_get_host(const tcp_listener_t *listener, tcp_error_t *errout);
uint16_t tcplistener_get_port(const tcp_listener_t *listener, tcp_error_t *errout);

size_t tcpstream_read(tcp_stream_t *me, uint8_t *buf, size_t bufsize, tcp_error_t *errout);
size_t tcpstream_write(tcp_stream_t *me, const uint8_t *buf, size_t bufsize, tcp_error_t *errout);
void tcpstream_flush(tcp_stream_t *me);

void socketaddr_free(socket_addr_t *addr);
void tcplistener_free(tcp_listener_t *me);
void tcpstream_free(tcp_stream_t *me);
void tcpconn_free(tcp_conn_t *me);

#endif // MSU_SOCKET_H