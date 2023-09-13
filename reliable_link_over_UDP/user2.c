#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdio.h>
#include<unistd.h>


#include "rsocket.h"

#define MY_PORT 30022
#define OTHER_PORT 30022


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
#define DEBUG 0

int main(int argc, char *argv[])
{
    int te, sockfd;
    char input_array[100];
    socklen_t size;

    struct sockaddr_in sample_test;
    struct sockaddr_in other;

    sample_test.sin_addr.s_addr = INADDR_ANY;
    sample_test.sin_port = htons(MY_PORT);
    sample_test.sin_family = AF_INET;
    

    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if(DEBUG >= 0 && sockfd < 0)
    {
        printf("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    if(DEBUG >= 0 && r_bind(sockfd, (struct sockaddr *)&sample_test, sizeof(sample_test)) < 0)
    {
        printf("Bind failed\n");
        exit(EXIT_FAILURE);
    }

    printf("_____IT HAS STARTED RECEVING_____\n");
  
    while(1)
    {
        
        te = r_recvfrom(sockfd, input_array, 100, 0, (struct sockaddr *)&other, &size);
        input_array[te] = '\0';
        printf("%s : From %s:%d\n", input_array, inet_ntoa(other.sin_addr), ntohs(other.sin_port));
    }
}
