#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <iostream>
using namespace std;
int defaultPort = 8000;
const string defaultIp = "127.0.0.1";

void usage()
{
	cout << "Usage : Please use either of following options..." << endl;
	cout << "./order_book_test \"order <orderId> buy/sell <qty> <price>\"" << endl;
	cout << "*ex:./order_book_test \"order 1001 buy 100 12.5\"" << endl;
	cout << "./order_book_test \"cancel <orderId>\"" << endl;
	cout << "*ex:./order_book_test \"cancel 1001\"" << endl;
	cout << "./order_book_test \"amend <orderId> <qty>\"" << endl;
	cout << "*ex:./order_book_test \"amend 1001 2000\"" << endl;
	cout << "./order_book_test \"q level ask <level>\"" << endl;
	cout << "*ex:./order_book_test \"q level ask 0\"" << endl;
	cout << "./order_book_test \"q level bid <level>\"" << endl;
	cout << "*ex:./order_book_test \"q level bid 1\"" << endl;
	cout << "./order_book_test \"q order <orderId>\"" << endl;
	cout << "*ex:./order_book_test \"q order 1001\"" << endl;
	cout << "./order_book_test \"display orders\"" << endl;
}

int main( int argc, char** argv )
{ 
	string ip, port, reqType;
	string request_message;
	if( argc < 2 )
	{
		usage();
		return -1;
	}
	
	//struct sockaddr_in address; 
	int sock = 0; 
	//int valread; 
	struct sockaddr_in serv_addr; 
	string requestMsg = string( argv[1] );

	if ( (sock = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) 
	{ 
		cout << "Socket creation error." << endl; 
		return -1; 
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons( defaultPort ); 

	if( inet_pton(AF_INET, defaultIp.c_str(), &serv_addr.sin_addr) <= 0 )
	{ 
		cout << "Address not supported." << endl;
		return -1; 
	} 

	// connect the socket 
	if( connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) < 0) 
	{ 
		cout << "Connection Failed." << endl;
		return -1; 
	} 
	char str[4096];
	strcpy( str, requestMsg.c_str() ); 
	
	send(sock, str, sizeof(str), 0); 
	//valread = read(sock, str, 4096); 
	read(sock, str, 4096); 
	
	cout << "*** Message from Server ***:" << str << endl;
	close( sock );
	return 0; 
} 

