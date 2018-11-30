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

class Client
{
	private:
		int sockfd; //socket file descriptor, and integer
		int domain; //communication domain e.g. AF_INET (IPv4), AF_INET6 (IPv6)
		int type;   //communication type e.g. TCP, UDP
		int proto;  //protocol value, e.g. IP (0).  This # appears on the protocol field of the IP header.
		int port; //port to bind

		sockaddr_in addr, listen_addr;

	public:
		
		Client(const int _domain, const int _type, const int _proto, std::string _addr, int _port):
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

			addr.sin_port = htons(_port);

			if((sockfd = socket(domain, type, proto)) < 0)
			{
				std::cout << "Creating socket file descriptor failed" << std::endl;
				throw sE;
			}

		}

		~Client()
		{
			std::cout << "Destructing, sockfd = " << sockfd << std::endl;
			close(sockfd);
			sockfd = -1;
		}

		int startConnect()
		{
		  if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
			{
				std::cout << "Socket connect failed!" << std::endl;
				return -1;
			}
			std::cout << "Connect successful!" << std::endl;

			return 0;
		}

		void sendMessage(std::string msg)
		{
			if(send(sockfd, msg.c_str(), msg.size(), 0) < 0)
			{
				std::cout << "Message send failed!" << std::endl;
			}
			std::cout << "Sent msg = \"" << msg << "\"" << std::endl;
		}
		
		void recvMessage()
		{
			int recvBuf[1024] = {0};
			int valread = read(sockfd, recvBuf, 1024);
			printf("%s\n", recvBuf);
		}
};

int main()
{
	Client *c = NULL;

	try
	{
		c = new Client(AF_INET, SOCK_STREAM, 0, "127.0.0.1", 8080);
	}
	catch(std::exception &e)
	{
		std::cout << "Socket creation failed!" << std::endl;
	}

	if(c->startConnect() < 0)
	{
		std::cout << "Connect failed!" << std::endl;
	}
	else
	{
		std::string msg("Hello");
		c->sendMessage(msg);
		c->recvMessage();
	}



	return 0;
}
