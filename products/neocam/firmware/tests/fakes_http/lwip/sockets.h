#ifndef FAKE_LWIP_SOCKETS_H
#define FAKE_LWIP_SOCKETS_H

#include <sys/socket.h>
#include <sys/select.h>

ssize_t fake_socket_send(int fd, const void *data, size_t length, int flags);
int fake_socket_select(int limit, fd_set *readable, fd_set *writable, fd_set *errors, struct timeval *timeout);
#define send fake_socket_send
#define select fake_socket_select

#endif
