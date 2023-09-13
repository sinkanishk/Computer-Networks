#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <dirent.h>

#define PORT 8080
#define BUFF_SIZE 1024
#define MAX 1024
#define MAX_CONN 1024
#define sz(a)  int((a).size())
#define pb push_back
#define all(c) (c).begin(),(c).end()
#define tr(c,i) for(typeof((c).begin() i = (c).begin(); i != (c).end(); i++)
#define present(c,x) ((c).find(x) != (c).end())
#define cpresent(c,x) (find(all(c),x) != (c).end())
#define forn(i,n) for(int i=0;i<n;++i)
#define foru(i,n) for(int i=1;i<=n;++i)
#define fortr(i,v) for(auto &i:v)
#define set(n) __builtin_popcount(n)
#define ll long long
#define DEF_PORT 1234
#define DEBUG 0
#define URL_SIZE 256

int max(int a, int b){if(a>b)return a; return b;}
int binds=0;
int sents=0;
int received=0;

void change(char *request_msg,int port)
{
    
    
    char *check=strstr(request_msg,"Host:");
    if(check!=NULL)
    {
            check=check+6;
            if((*check)<='9'&&(*check)>='0')
	    {
	    	
		    char prt[50];
		    sprintf(prt, "%d", port);        
		    int i=0;
		    char* start= strstr(request_msg,"http");
		    char* end=strstr(request_msg,prt);
		    end=end+4;
		    char ans[1024];
		    for(char*iter=request_msg;iter!=start;++iter)
		    {
			ans[i++]=*iter;
		    }
		    for(char*iter=end;*iter!='\0';++iter)
		    {
			ans[i++]=*iter;
		    }
		    ans[i]='\0';
		    for(i=0;ans[i]!='\0';++i)
		    {
			request_msg[i]=ans[i];
		    }
		    request_msg[i]='\0';
	   }
   }

}

int parse_request (char *request_msg,char *ip_add) 
{
    char tmp_url[URL_SIZE];
    int port;
    int j = 0;
    int i;
    for (i=0; i<15 && request_msg[i] && request_msg[i]!=' '; i++);
    i++;
    for (; request_msg[i] && request_msg[i]==' '; i++);    
    for (; request_msg[i] && request_msg[i]!=' '; i++)
        tmp_url[j++] = request_msg[i];
    tmp_url[j] = '\0';
    int k = 0, l = 0;
    if (j > 4 && (strncmp (tmp_url, "http://", 7) == 0) )
    {
        while (tmp_url[k]!=':') k++;
        k+=3;   
        while (k < URL_SIZE && tmp_url[k] && tmp_url[k]!=':' && tmp_url[k]!='/' && tmp_url[k]!=' ') ip_add[l++] = tmp_url[k++];
    } 
    else 
    {   
        while (k < URL_SIZE && tmp_url[k] && tmp_url[k]!=':' && tmp_url[k]!=' ') ip_add[l++] = tmp_url[k++];
    }
    ip_add[l] = '\0';
    l = 0;
    if (tmp_url[k]==':') 
    {
        k++;
        char temp[64];
        for (; k < URL_SIZE && tmp_url[k] && tmp_url[k]!=' '; k++) temp[l++] = tmp_url[k];
        temp[l]='\0';
        port = atoi (temp);
    } 
    else 
    {
        port = 80;
    }
    return port;

}



int main(int argc, char *argv[]) 
{
    fflush(NULL);
    signal(SIGPIPE, SIG_IGN);
    if(DEBUG)
    {
	printf("I am here 1\n ");
    }
    char ip_add[1024];
   
    char request[MAX_CONN][6000];   // temporary buffer
    memset(request, '\0', sizeof(request));


    if (argc != 2) 
    {
        printf("Correct usage : ./SimHTTPProxy <listen port>\n");
        return 0;
    }

    uint16_t LISTEN_PORT = atoi(argv[1]);

    struct sockaddr_in proxyaddr;
    struct sockaddr_in  servaddr_browser;
    struct sockaddr_in cliaddr;
    socklen_t len, len_cli;

    int tcpfd;
    int maxfd;
    int  flag;

    memset(&servaddr_browser, 0, sizeof(servaddr_browser));


    char buff[BUFF_SIZE];
    memset(buff, 0, sizeof(buff));

    // Create tcp socket file descriptor
    if ((tcpfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket in creation failed\n");
        exit(EXIT_FAILURE);
    }
    ++binds;
    if (fcntl(tcpfd, F_SETFL, O_NONBLOCK) < 0) 
    {
        perror("Could not make non-blocking socket\n");
        exit(EXIT_FAILURE);
    }
    
    if((setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0))
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    servaddr_browser.sin_family = AF_INET;
    servaddr_browser.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_browser.sin_port = htons(LISTEN_PORT);

    ++binds;
    // Attach socket to port 8080
    if (bind(tcpfd, (struct sockaddr*)&servaddr_browser, sizeof(servaddr_browser)) < 0)
    {
        perror("In socket Bind failure\n");
        exit(EXIT_FAILURE);
    }
    printf("\033[1;32m/////////////// Proxy Server Binded ///////////////\033[0m\n\n");

    // Marks server_fd as a passive socket, now ready to accept incoming connections using connect()
    if (listen(tcpfd, MAX_CONN) < 0)
    {
        perror("Sock in Listen failure\n");
        exit(EXIT_FAILURE);
    }
    printf("\033[1;32m//////////////  Listening to the connections ///////////////\033[0m\n\n");
	
    fd_set read_fd;
    fd_set write_fd;
    if(DEBUG)
    {
	printf("I am here 2\n ");
    }
    FD_ZERO(&read_fd);
    FD_ZERO(&write_fd);

    int browser_fd[MAX_CONN], inst_proxy_fd[MAX_CONN], num_conn = 0;

    while (1) 
    {

        FD_ZERO(&read_fd);
        FD_ZERO(&write_fd);
        FD_SET(STDIN_FILENO, &read_fd);
        maxfd = 1;
        FD_SET(tcpfd, &read_fd);
        maxfd = tcpfd;
	if(DEBUG)
    	{
		printf("I am here 3\n ");
    	}
        // add all connections to FD SET
        for (int i = 0; i < num_conn; i++) 
        {
            FD_SET(browser_fd[i], &write_fd);
            FD_SET(inst_proxy_fd[i], &write_fd);
	    if(DEBUG)
    		{
		printf("I am here i+3\n ");
    		}
	    FD_SET(browser_fd[i], &read_fd);
            FD_SET(inst_proxy_fd[i], &read_fd);
           
            maxfd = max(max(maxfd, browser_fd[i]), inst_proxy_fd[i] );
        }
        maxfd = maxfd + 1;

        
        flag = select(maxfd, &read_fd, &write_fd, NULL, NULL);
	++binds;
        if (flag > 0) 
        {
            if(FD_ISSET(tcpfd, &read_fd))    // New connection
            {
                if (num_conn >= MAX_CONN || (browser_fd[num_conn] = accept(tcpfd, (struct sockaddr *)&cliaddr, &len_cli)) < 0) 
                {
                    perror("Tcp connection failed\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    char *cli_ip; 
                    cli_ip = strdup (inet_ntoa (cliaddr.sin_addr));
                    int cli_Port = (int) ntohs(cliaddr.sin_port);   
		    if(DEBUG)
    		    {
			printf("I am here 5\n ");
    		    }    
                    num_conn++;
                }
                
            }

            else if(FD_ISSET(STDIN_FILENO, &read_fd))   // If something is typed in the terminal
            {
                char cmd[MAX];
                scanf("%s", cmd);
		if(DEBUG)
    		{
			printf("I am here 6\n ");
    		}
                if(strcmp(cmd, "exit") == 0)
                {
                    for(int i = 0 ; i < num_conn ; i++)
                    {
			close(browser_fd[i]);                       
			 close(inst_proxy_fd[i]);
                        
                    }
		    --binds;
                    close(tcpfd);
                    exit(EXIT_SUCCESS);
                }
            }

            for (int i = 0; i < num_conn; i++) 
            {
                char buff[MAX];
                int a, b;
		++binds;
                if(FD_ISSET(browser_fd[i], &read_fd))    // new incoming http request
                {
                    a = read(browser_fd[i], buff, MAX);
                    if(!a)                          
                        continue;
		    if(DEBUG)
    			{
				printf("I am here 8\n ");
    			}
                    strcat(request[i], buff);  
		    ++binds;        
                    if(strstr(buff, "\r\n\r\n") != NULL)
                    {

                        int port = parse_request(request[i],ip_add);
			change(request[i],port);
                        struct hostent *hn = gethostbyname(ip_add);
			
                        if(hn == NULL)
                        {
                            perror("invalid host");
                            exit(EXIT_FAILURE);
                        }

			if(DEBUG)
	    		{
				printf("I am here 11\n ");
	    		}
                        char ip[MAX];
                        strcpy(ip, inet_ntoa(*( struct in_addr*)hn->h_addr));
                        printf("\033[1;32mResolved ip= %s, port= %d\033[0m\n",ip,port);
			 if(DEBUG)
    			{
				printf("I am here 8\n ");
    			}
                        //tcp connection
                        if ((inst_proxy_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
                        {
                            fprintf(stdout, "socket() failed: %s\n", strerror(errno));
                            exit(0);
                        }
			++binds;
                        if (fcntl(inst_proxy_fd[i], F_SETFL, O_NONBLOCK) < 0) 
                        {
                            fprintf(stdout, "fcntl() failed: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }

                        struct sockaddr_in addr;
                        addr.sin_family = AF_INET;
			++binds;
			if(DEBUG)
    			{
				printf("I am here 9\n ");
    			}
                        addr.sin_port = htons(port);
			++binds;
                        if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
                        {
                            perror("Invalid proxy server address\n");
                            exit(EXIT_FAILURE);
                        }
                        // cannot connect immediately, needs some time
                        connect(inst_proxy_fd[i], (struct sockaddr *)&addr, sizeof(addr));
                    }
                    memset(buff, 0, MAX);

                }
                
                if(FD_ISSET(inst_proxy_fd[i], &write_fd))      // HTTP server ready to accept requests
                {
		    b = write(inst_proxy_fd[i], request[i], strlen(request[i]));
		    ++sents;
		    if(DEBUG)
    		    {
			printf("I am here 10\n ");
    		    }
                    memset(request[i], '\0', sizeof(request[i]));
                    if (errno == EPIPE) 
                    {
                        continue;
                    }
                    
                }
                if(FD_ISSET(inst_proxy_fd[i], &read_fd) && FD_ISSET(browser_fd[i], &write_fd))  // Send http response from server to client
                {
                    memset(buff, 0, MAX);
                    a = read(inst_proxy_fd[i], buff, MAX);
		    ++received;
		    if(DEBUG)
    		    {
			printf("I am here 1\n ");
    		    }
                    if(a)
                    {
                        b = send(browser_fd[i], buff, a, 0);
			++sents;
                        if (errno == EPIPE) 
                        {
                            close(browser_fd[i]);
                            continue;
                        }
                    }   
                }
            }
        }
    }
    return 0;
}
