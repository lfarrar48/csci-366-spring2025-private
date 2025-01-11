#include "msulib/socket.h"
#include <assert.h>

const char *tcp_error_message(tcp_error_t err) {
    if (err == TCP_ERROR_NONE) return "(nil)";
    if (err == TCP_ERROR_INIT) return "unable to initialize sockets";
    if (err == TCP_ERROR_ADDR_RESOLUTION) return "unable to resolve address";
    if (err == TCP_ERROR_INVALID_ADDR) return "invalid address provided";
    if (err == TCP_ERROR_SOCKET_CREATION) return "unable to create socket";
    if (err == TCP_ERROR_SOCKET_BIND) return "unable to bind socket";
    if (err == TCP_ERROR_SOCKET_LISTEN) return "unable to listen on socket";
    if (err == TCP_ERROR_SOCKET_ACCEPT) return "unable to accept incoming connection";
    if (err == TCP_ERROR_SOCKET_READ) return "unable to read from socket";
    if (err == TCP_ERROR_SOCKET_WRITE) return "unable to write to socket";
    if (err == TCP_ERROR_CLOSED) return "socket closed unexpectedly";
    return "BAD ERROR, IDK WHAT HAPPENED";
}

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#   pragma comment(lib, "ws2_32.lib")
#   include <winsock2.h>
#   include <ws2tcpip.h>
typedef int socklen_t;
#else
#   include <sys/socket.h>
#   include <netdb.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   define SOCKET int
#   define INVALID_SOCKET (-1)
#   define SOCKET_ERROR (-1)
#   define closesocket close
#endif


struct socketaddr {
    struct sockaddr_in addr;
};

struct tcplistener {
    SOCKET socket;
};

struct tcpstream {
    SOCKET socket;
};

void sockets_init(tcp_error_t *errout) {
#ifndef _WIN32
    (void) errout;
#else
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        *errout = TCP_ERROR_INIT;
    }
#endif
}

void sockets_cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}

socket_addr_t *socket_resolve_addr(const char *host, uint16_t port, tcp_error_t *errout) {
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *info;
    char ports[6];
    snprintf(ports, sizeof(ports), "%u", port);

    int result = getaddrinfo(host, ports, &hints, &info);
    if (result != 0) {
        *errout = TCP_ERROR_ADDR_RESOLUTION;
        return NULL;
    }

    socket_addr_t *addr = malloc(sizeof(socket_addr_t));
    if (!addr) {
        freeaddrinfo(info);
        *errout = TCP_ERROR_INVALID_ADDR;
        return NULL;
    }

    struct sockaddr_in *addr_in = (struct sockaddr_in *) info->ai_addr;
    addr->addr = *addr_in;

    freeaddrinfo(info);
    *errout = TCP_ERROR_NONE;
    return addr;
}

tcp_listener_t *tcp_bind(socket_addr_t *addr, tcp_error_t *errout) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        *errout = TCP_ERROR_SOCKET_CREATION;
        return NULL;
    }

    int bind_result = bind(sock, (struct sockaddr *)&addr->addr, sizeof(addr->addr));
    if (bind_result == SOCKET_ERROR) {
        closesocket(sock);
        *errout = TCP_ERROR_SOCKET_BIND;
        return NULL;
    }

    int listen_result = listen(sock, SOMAXCONN);
    if (listen_result == SOCKET_ERROR) {
        closesocket(sock);
        *errout = TCP_ERROR_SOCKET_LISTEN;
        return NULL;
    }

    tcp_listener_t *listener = malloc(sizeof(tcp_listener_t));
    if (!listener) {
        closesocket(sock);
        assert(0 && "out of memory!\n");
    }

    listener->socket = sock;
    *errout = TCP_ERROR_NONE;
    return listener;
}

tcp_conn_t tcp_accept(tcp_listener_t *server, tcp_error_t *errout) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    SOCKET client_socket = accept(server->socket, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_socket == INVALID_SOCKET) {
        *errout = TCP_ERROR_SOCKET_ACCEPT;
        return (tcp_conn_t){NULL, NULL};
    }

    tcp_stream_t *stream = malloc(sizeof(tcp_stream_t));
    if (!stream) {
        closesocket(client_socket);
        assert(0 && "out of memory!\n");
    }

    socket_addr_t *peer_address = malloc(sizeof(socket_addr_t));
    if (!peer_address) {
        closesocket(client_socket);
        free(stream);
        assert(0 && "out of memory!\n");
    }

    stream->socket = client_socket;
    peer_address->addr = client_addr;
    *errout = TCP_ERROR_NONE;
    return (tcp_conn_t){
        .stream = stream,
        .peer_address = peer_address
    };
}

const msu_str_t *tcplistener_get_host(const tcp_listener_t *listener, tcp_error_t *errout) {
    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);

    int result = getsockname(listener->socket, (struct sockaddr*)&addr, &addr_len);
    if (result == SOCKET_ERROR) {
        *errout = TCP_ERROR_ADDR_RESOLUTION;
        return NULL;
    }

    char* host = (char*)malloc(INET6_ADDRSTRLEN);
    assert(host && "out of memory!\n");

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), host, INET6_ADDRSTRLEN);
    } else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), host, INET6_ADDRSTRLEN);
    } else {
        *errout = TCP_ERROR_ADDR_RESOLUTION;
        free(host);
        return NULL;
    }

    return __msu_str_new_unsafe(host);
}

uint16_t tcplistener_get_port(const tcp_listener_t *listener, tcp_error_t *errout) {
    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);

    int result = getsockname(listener->socket, (struct sockaddr*)&addr, &addr_len);
    if (result == SOCKET_ERROR) {
        *errout = TCP_ERROR_ADDR_RESOLUTION;
        return 0;
    }

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)&addr;
        return ntohs(ipv4->sin_port);
    } else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)&addr;
        return ntohs(ipv6->sin6_port);
    } else {
        *errout = TCP_ERROR_ADDR_RESOLUTION;
        return 0;
    }
}

size_t tcpstream_read(tcp_stream_t *me, uint8_t *buf, size_t bufsize, tcp_error_t *errout) {
    size_t nread = recv(me->socket, (char *)buf, (int)bufsize, 0);
    if (nread == SOCKET_ERROR) {
        *errout = TCP_ERROR_SOCKET_READ;
        return 0;
    }

    *errout = TCP_ERROR_NONE;
    return nread;
}

size_t tcpstream_write(tcp_stream_t *me, const uint8_t *buf, size_t bufsize, tcp_error_t *errout) {
    size_t sent = 0;
    while (sent < bufsize) {
        size_t result = send(me->socket, (const char *) buf, (int) bufsize, 0);
        if (result == SOCKET_ERROR) {
            *errout = TCP_ERROR_SOCKET_WRITE;
            return sent;
        }

        if (result == 0) {
            *errout = TCP_ERROR_CLOSED;
            return sent;
        }

        sent += result;
    }
    return sent;
}

void socketaddr_free(socket_addr_t *addr) {
    if (addr) {
        free(addr);
    }
}

void tcplistener_free(tcp_listener_t *me) {
    if (me) {
        closesocket(me->socket);
        free(me);
    }
}

void tcpstream_free(tcp_stream_t *me) {
    if (me) {
        closesocket(me->socket);
        free(me);
    }
}

void tcpconn_free(tcp_conn_t *me) {
    if (!me) return;
    tcpstream_free(me->stream);
    socketaddr_free(me->peer_address);
}
