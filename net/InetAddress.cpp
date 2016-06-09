//
// Created by root on 16-6-9.
//

#include "../net/InetAddress.h"
#include "../base/Logging.h"
#include "../net/Endian.h"
#include "../net/SocketsOps.h"

//Unix和Linux特有的头文件，主要定义了与网络有关的结构、变量类型、宏、函数等。
//
//通过IP地址获得主机有关的网络信息
//struct hostent*gethostbyaddr(const void *addr, size_t len, int type);
//通过主机名获得主机的网络信息*/
//struct hostent*gethostbyname(const char *name);
#include <netdb.h>
#include <strings.h>  // bzero
#include <netinet/in.h>

#include <boost/static_assert.hpp>

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

using namespace salangane;
using namespace salangane::net;
BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof(struct sockaddr_in6));
BOOST_STATIC_ASSERT(offsetof(sockaddr_in, sin_family) == 0);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in6, sin6_family) == 0);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in, sin_port) == 2);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in6, sin6_port) == 2);

#if !(__GNUC_PREREQ (4,6))
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif


InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
    BOOST_STATIC_ASSERT(offsetof(InetAddress, addr6_) == 0);
    BOOST_STATIC_ASSERT(offsetof(InetAddress, addr_) == 0);
    if (ipv6) {
        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::hostToNetwork16(port);
    } else {
        bzero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
        addr_.sin_port = sockets::hostToNetwork16(port);
    }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        bzero(&addr6_, sizeof addr6_);
        sockets::fromIpPort(ip.c_str(), port, &addr6_);
    } else {
        bzero(&addr_, sizeof addr_);
        sockets::fromIpPort(ip.c_str(), port, &addr_);
    }
}

string InetAddress::toIpPort() const {
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

string InetAddress::toIp() const {
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

uint32_t InetAddress::ipNetEndian() const {
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::toPort() const {
    return sockets::networkToHost16(portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress* out) {
    assert(out != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    bzero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer,
                              sizeof (t_resolveBuffer), &he, &herrno);
    if (ret == 0 && he != NULL) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    } else {
        if (ret) {
            LOG_SYSERR << "InetAddress::resolve";
        }
        return false;
    }
}


//struct hostent *gethostbyname(const char *name);
//这个函数的传入值是域名或者主机名，例如"www.google.com","wpc"等等。
//传出值，是一个hostent的结构（如下）。如果函数调用失败，将返回NULL。
//struct hostent {
//    char *h_name; 表示的是主机的规范名。例如www.google.com的规范名其实是www.l.google.com。
//    char **h_aliases; 表示的是主机的别名。www.google.com就是google他自己的别名。有的时候，有的主机可能有好几个别名，这些，其实都是 为了易于用户记忆而为自己的网站多取的名字。
//    int h_addrtype; 表示的是主机ip地址的类型，到底是ipv4(AF_INET)，还是ipv6(AF_INET6)
//    int h_length; 表示的是主机ip地址的长度
//    char **h_addr_list; 表示的是主机的ip地址，注意，这个是以网络字节序存储的。
//    千万不要直接用printf带%s参数来打这个东西，会有问题的哇。所以到真正需要打印出这个IP的话，需要调用inet_ntop()。
//        const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt) ：
//        这个函数，是将类型为af的网络地址结构src，转换成主机序的字符串形式，存放在长度为cnt的字符串中。
//        这个函数，其实就是返回指向dst的一个指针。如果函数调用错误，返回值是NULL。
//};
//在多线程里面进行域名解析的时候，总是会有一些问题，
// 好像gethostbyname是线程不安全的
// 因为gethostbyname返回的是一个指向静态变量的指针 不可重入
// 很可能刚要读时值就被其它线程修改
// 所以 新的posix中增加了另一个可重入的从主机名(域名) gethostbyname_r