#include <netdb.h>
#include <sys/socket.h>

extern "C" int gethostbyaddr_r(const char* addr, socklen_t len, int type,
        struct hostent *ret, char *buf, size_t buflen,
        struct hostent **result, int *h_errnop) {
	hostent* host = gethostbyaddr(addr, len, type);
	if(host)
		*ret = *host;
	*result = host;
	return host == 0;
}