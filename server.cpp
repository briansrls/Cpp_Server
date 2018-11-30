#include <iostream>
#include <string>
#include <cstring>
#include <exception>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>
#include <netdb.h>

class socketException: public std::exception
{
		const char* what() const throw() 
		{
			std::string s = std::to_string(errno);
			const char *pchar = s.c_str();
			return pchar; 
		}
} sE;

class Server
{
	private:
		int sockfd; //socket file descriptor, and integer
		int domain; //communication domain e.g. AF_INET (IPv4), AF_INET6 (IPv6)
		int type;   //communication type e.g. TCP, UDP
		int proto;  //protocol value, e.g. IP (0).  This # appears on the protocol field of the IP header.
		int port; //port to bind

		int clientfd;

		sockaddr_in addr, listen_addr;

	public:
		
		Server(const int _domain, const int _type, const int _proto, std::string _addr, int _port):
			clientfd(-1),
			sockfd(-1),
			domain(_domain),
			type(_type),
			proto(_proto),
			port(_port)
		{
			int opt = 1;

			memset(&addr, 0, sizeof(addr));
			addr.sin_family = _domain;

			hostent *host;
			if ((host = gethostbyname(_addr.c_str())) == NULL)
			{
				std::cout << "Unable to find hostname!" << std::endl;
				throw sE;
			}

			addr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
			//addr.sin_addr.s_addr = htonl(INADDR_ANY);

			//std::cout << "Address found is " << addr.sin_addr.s_addr << std::endl;

			addr.sin_port = htons(_port);

			if((sockfd = socket(domain, type, proto)) == -1)
			{
				std::cout << "Creating socket file descriptor failed" << std::endl;
				throw sE;
			}

			if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
			{
				close(sockfd);
				std::cout << "Setting socket options failed" << std::endl;
				throw sE;
			}

			if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
			{
				close(sockfd);
				std::cout << "Socket bind failed" << std::endl;
				throw sE;
			}

		}

		~Server()
		{
			std::cout << "Destructing, sockfd = " << sockfd << std::endl;
			close(sockfd);
			sockfd = -1;
		}

		int startListen(int _backlog)
		{
			int newsockfd;
			socklen_t listenlen = sizeof(listen_addr);
			int recvbuf[1024] = {0};

			if(listen(sockfd, _backlog) == -1)
			{
				close(sockfd);
				std::cout << "Listening to socket failed!" << std::endl;
				return -1;
			}

			std::cout << "Listen successful, process going to sleep to await new connections" << std::endl;

			if((clientfd = accept(sockfd, (struct sockaddr *)&listen_addr, &listenlen)) == -1)
			{
				close(sockfd);
				std::cout << "Accepting connection failed!" << std::endl;
				return -1;
			}

			std::cout << "newsockfd = " << clientfd << std::endl;

			read( clientfd, recvbuf, 1024);

			printf("%s\n", recvbuf);

			
			return 0;
		}

		int startSend(const std::string &msg)
		{
			if(send(clientfd, msg.c_str(), msg.size(), 0) < 0)
			{
				std::cout << "Message send failed!" << std::endl;
			}
			std::cout << "Sent msg \"" << msg << "\"" << std::endl;
		}
};

int main()
{
	Server *s = NULL;
	try
	{
		s = new Server(AF_INET, SOCK_STREAM, 0, "127.0.0.1", 8080); 
	}
	catch(std::exception &e)
	{
		std::cout << "Construction failed! errno=" << e.what() << std::endl;
	}

	if(s->startListen(3) < 0)
	{
		std::cout << "Listen failed!" << std::endl;
	}

	std::string msg("Hello!");
	if(s->startSend(msg) < 0)
	{
		std::cout << "Send failed!" << std::endl;
	}



	return 0;
}
