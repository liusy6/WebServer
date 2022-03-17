#pragma once
#include <cstdlib>
#include <string>

void handle_for_sigpipe();
int setSocketNonBlocking(int fd);
int socket_bind_listen(int port);
void shutDownWR(int fd);
void setSocketNodelay(int fd);
