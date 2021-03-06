#ifndef __TCP__
#define __TCP__

#include <stdint.h>

#ifdef __WINDOWS__
    #include <Windows.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

namespace robot
{

#ifdef __WINDOWS__
inline void delay( unsigned long ms )
{
    Sleep(ms);
}
#else
#include <unistd.h>

inline void delay( unsigned long ms )
{
    usleep(ms * 1000);
}
#endif

// only for tests!!!
namespace tcp_test
{

class TCPInterface
{
#ifdef __WINDOWS__
    uint16_t wsa_version;
    typedef SOCKET socket_t;
#else
    typedef int socket_t;
#endif
    uint32_t ip_addr;
    uint16_t port;
    bool is_ready;

    socket_t io_socket;
    socket_t listener_socket;
    
    sockaddr_in my_addr, client_addr;

    int tcp_connect()
    {
        io_socket = socket(AF_INET, SOCK_STREAM, 0);
        my_addr.sin_addr.s_addr = ip_addr;
        my_addr.sin_port = htons(port);
        my_addr.sin_family = AF_INET;

        if(!connect(io_socket,(sockaddr*)&my_addr, sizeof(my_addr)))
            return 0;

        return 1;
    }

public:
    TCPInterface(uint16_t p = 8101):
#ifdef __WINDOWS__
        wsa_version(0x0202),
        ip_addr(inet_addr("127.0.0.1")),
#else
        ip_addr(htonl(INADDR_LOOPBACK)),
#endif
        port(p),
        is_ready(false)
    {}
    void client_connect ()
    {
#ifdef __WINDOWS__
        WSAData dt;
        WSAStartup(wsa_version,&dt);
#endif
        tcp_connect();
        is_ready = true;
    }

    void server_start()
    {
        listener_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(listener_socket < 0)
            return;
    
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(5200);
        client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(listener_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0)
            return;

        listen(listener_socket, 1);

        io_socket = accept(listener_socket, NULL, NULL);
        if(io_socket < 0)
            return;

        is_ready = true;
        std::cout << "ready " << this << std::endl;
    }

    uint32_t write(const char* write_buffer, int length)
    {
        if(!is_ready)
            return -2;
        
        uint32_t send_count = send(io_socket, (char*)write_buffer, length, 0);

        if(!(int)send_count)
            return -1;
        
        return send_count;
    }

    uint32_t read (uint8_t* read_buffer,  int length)
    {
        if(!is_ready) {
            std::cout << "not ready " << this << std::endl;
            return -2;
        }

        int bytesRW = 0;

        if (length > 0) bytesRW = recv(io_socket,(char*)read_buffer,length,0);
        if(!(int)bytesRW)
        {
            return -1;
        }
        return bytesRW;
    }
};

}

}

#endif
