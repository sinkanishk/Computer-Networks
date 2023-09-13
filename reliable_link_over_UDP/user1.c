#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdio.h>

#include "rsocket.h"

#define OTHER_PORT 30022
#define MY_PORT 30023
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
    int n, i, sockfd;
    char input[100];
    printf("Enter the string you want to send\n");
    scanf("%s", input);
    printf("Your protocol is preparing to send...\n");
    n = strlen(input);

    struct sockaddr_in sample_test;
    struct sockaddr_in other;
    struct sockaddr_in temp;
    sample_test.sin_family = AF_INET;
    other.sin_family = AF_INET;
    
    sample_test.sin_addr.s_addr = INADDR_ANY;
    other.sin_addr.s_addr = INADDR_ANY;
    
    sample_test.sin_port = htons(MY_PORT);
    other.sin_port = htons(OTHER_PORT);

    sockfd = r_socket(AF_INET, SOCK_MRP, 0);

    if(DEBUG>=0 && sockfd < 0)
    {
        printf("We are sorry but Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    if(DEBUG>=0 && r_bind(sockfd, (struct sockaddr *)&sample_test, sizeof(sample_test)) < 0)
    {
        printf("We are sorry but Bind got failed\n");
        exit(EXIT_FAILURE);
    }

    for(i=0;i<n;)
    {
        r_sendto(sockfd, input + i, 1, 0, (struct sockaddr *)&other, sizeof(other));
	i=i+1;
        usleep(300000);
    }
    while(1);

}
