/*
 A simple server in the internet domain using TCP
 The port number is passed as an argument
 This version runs forever, forking off a separate
 process for each connection
 */

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>  /* for the waitpid() system call */
#include <signal.h>  /* signal name macros, and the kill() prototype */
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>


/* REQUEST MESSAGE
 
 GET /test.html HTTP/1.1
 Host: localhost:8083
 User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.10; rv:41.0) Gecko/20100101 Firefox/41.0
 Accept: text/html,application/xhtml+xml,application/xml;q=0.9,* / *;q=0.8
 Accept-Language: en-US,en;q=0.5
 Accept
 
 */

/* RESPONSE MESSAGE
 
 HTTP/1.1 200 OK
 Connection: close
 
 Date: Tue, 09 Aug 2011 15:44:04 GMT
 Server: Apache/2.2.3 (CentOS)
 Last-Modified: Tue, 09 Aug 2011 15:11:03 GMT
 Content-Length: 6821
 Content-Type: text/html
 (data data data data data ...)
 
 ---
 
 HTTP/1.0 200 OK
 Content-Type: image/png
 Date: Sun, 18 Oct 2015 01:36:58 GMT
 Expires: Sun, 18 Oct 2015 01:36:58 GMT
 Cache-Control: private, max-age=31536000
 Last-Modified: Fri, 04 Sep 2015 22:33:08 GMT
 X-Content-Type-Options: nosniff
 Server: sffe
 Content-Length: 13504
 X-XSS-Protection: 1; mode=block
 
 
 HTTP/1.0 404 Not Found
 Content-Type: text/html; charset=UTF-8
 X-Content-Type-Options: nosniff
 Date: Sun, 18 Oct 2015 01:29:40 GMT
 Server: sffe
 Content-Length: 1571
 X-XSS-Protection: 1; mode=block
 
 
 HTTP/1.1 404 Not Found
 Server: GitHub.com
 Date: Sun, 18 Oct 2015 01:36:46 GMT
 Content-Type: text/html; charset=utf-8
 Content-Length: 9116
 Connection: close
 ETag: "5519ee21-239c"
 Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'; img-src data:; connect-src 'self'
 */

//http://stackoverflow.com/questions/9828752/read-line-by-line-from-a-socket-buffer

// http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
// CTRL+F to "Enhancements to the server code" to see the dostuff() function to let the connection actually run forever

void printSomething(){
  printf("hello");
}

int get_word(int offset, char *line, char** word)
{
    //memset(word,0,100);
    int position = 0;
    char c = line[offset+position];
    while (c != ' ' && c != '\r')  // the NULL is preventing this from being entered
    {
        //printf("%c", c);
        c = line[offset+position];
        word[position] = c;
        position++;
    }
    //printf("**** %s", word);
    return offset+position;
}

void parse(char *buffer, char** response_buffer)
{
    //printf("world");
    char *line;
    line = (char*) malloc(1000); 

    line = strtok(buffer, "\n");

    // printf("%c", line[0]); // G
    // printf("%c", line[1]);
    // printf("%c", line[2]);  // T
    // printf("%c", line[3]);
    // printf("%c", line[4]);  // /
    // printf("%c", line[5]);  // t
    // printf("%c", line[6]);
    // printf("%c", line[7]);   // s
    // printf("%c", line[8]);    // t
    // printf("%c", line[9]);
    // printf("%c", line[10]);   // h
    // printf("%c", line[11]);
    // printf("%c", line[12]);
    // printf("%c", line[13]);
    // printf("%c", line[14]);

    // printf("%c", line[15]);   // H
    // printf("%c", line[16]);
    // printf("%c", line[17]);
    // printf("%c", line[18]);   // P
    // printf("%c", line[19]);   // / 
    // printf("%c", line[20]);
    // printf("%c", line[21]);
    // printf("%c", line[22]);   // 1

    // Request line
    int word_start = 0, word_end = 0;
    char req_type[100];
    word_end = get_word(word_start, line, &req_type);
    word_start = word_end + 1;
    
    char file_name[100]; // has a slash as first char
    word_end = get_word(word_start, line, &file_name);
    word_start = word_end;
    //memmove(file_name, file_name+1, strlen(file_name)); // should no longer have slash

    char html_version[100];
    word_end = get_word(word_start, line, &html_version);

    //printf("request: %s, file: %s, html version: %s", req_type, file_name, html_version);
    
    
    //     HTTP/1.0 404 Not Found
    // Content-Type: text/html; charset=UTF-8
    // X-Content-Type-Options: nosniff
    // Date: Sun, 18 Oct 2015 01:29:40 GMT
    // Server: sffe
    // Content-Length: 1571
    // X-XSS-Protection: 1; mode=block
    
    
    // HTTP/1.1 404 Not Found
    // Server: GitHub.com
    // Date: Sun, 18 Oct 2015 01:36:46 GMT
    // Content-Type: text/html; charset=utf-8
    // Content-Length: 9116
    // Connection: close
    // ETag: "5519ee21-239c"
    // Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'; img-src data:; connect-src 'self'

    // int j = 0;
    // while(j < sizeof(file_name)){
    //   printf("%c", file_name[j]);
    //   j = j + 1;
    // }

    // OK, now we are correctly entering this when the file doesn't exist.
    if (access(file_name, F_OK) == -1)    //FILE DOESN'T EXIST
    {

        strncpy(*response_buffer, "HTTP/1.1", 8);    
        strcat(*response_buffer, " 404 Not Found\r\n");
        strcat(*response_buffer, "Server: CS118 Project\r\n");

        // Date and Time
        time_t t = time(NULL);
        struct tm * date_and_time;
        // date_and_time = localtime(&t);
        date_and_time = gmtime(&t);

        strcat(*response_buffer, "Date: ");

        //printf("\n WEEKDAY INT: %d\n", date_and_time->tm_wday);

        // Day of the week
        switch (date_and_time->tm_wday) {
        case 0:
            strcat(*response_buffer, "Sun, ");
            break;
        case 1:
            strcat(*response_buffer, "Mon, ");
            break;
        case 2:
            strcat(*response_buffer, "Tue, ");
            break;
        case 3:
            strcat(*response_buffer, "Wed, ");
            break;
        case 4:
            strcat(*response_buffer, "Thu, ");
            break;
        case 5:
            strcat(*response_buffer, "Fri, ");
            break;
        case 6:
            strcat(*response_buffer, "Sat, ");
            break;
        }
        
        // Date of the month
        if (date_and_time->tm_mday < 10) {
            strcat(*response_buffer, "0");
        }

        char int_string[5];  // To convert integers to strings (year = need 5)
        int n;
        n = sprintf(int_string, "%d", date_and_time->tm_mday);
        strcat(*response_buffer, int_string);  // change second argument to string

      
        // Month
        switch (date_and_time->tm_mon) {
        case 0:
            strcat(*response_buffer, " Jan ");
            break;
        case 1:
            strcat(*response_buffer, " Feb ");
            break;
        case 2:
            strcat(*response_buffer, " Mar ");
            break;
        case 3:
            strcat(*response_buffer, " Apr ");
            break;
        case 4:
            strcat(*response_buffer, " May ");
            break;
        case 5:
            strcat(*response_buffer, " Jun ");
            break;
        case 6:
            strcat(*response_buffer, " Jul ");
            break;
        case 7:
            strcat(*response_buffer, " Aug ");
            break;
        case 8:
            strcat(*response_buffer, " Sep ");
            break;
        case 9:
            strcat(*response_buffer, " Oct ");
            break;
        case 10:
            strcat(*response_buffer, " Nov ");
            break;
        case 11:
            strcat(*response_buffer, " Dec ");
            break;
        }
        
        // Year
        n = sprintf(int_string, "%d", date_and_time->tm_year + 1900);  // Epoch time
        strcat(*response_buffer, int_string);
        
        // Hour
        strcat(*response_buffer, " ");
        if (date_and_time->tm_hour < 10) {
            strcat(*response_buffer, "0");
        }
        n = sprintf(int_string, "%d", date_and_time->tm_hour);
        strcat(*response_buffer, int_string);
        
        // Minutes
        strcat(*response_buffer, ":");
        if (date_and_time->tm_min < 10) {
            strcat(*response_buffer, "0");
        }
        n = sprintf(int_string, "%d", date_and_time->tm_min);
        strcat(*response_buffer, int_string);
        
        // Seconds
        strcat(*response_buffer, ":");
        if (date_and_time->tm_sec < 10) {
            strcat(*response_buffer, "0");
        }
        n = sprintf(int_string, "%d", date_and_time->tm_sec);
        strcat(*response_buffer, int_string);
        strcat(*response_buffer, " GMT\r\n");

        // Content Type
        int i;
        for (i=0; i < sizeof(file_name); i+=sizeof(char*)) {
            if (file_name[i] == '.') {
                switch (file_name[i+sizeof(char*)]) {
                case 'j':
                    strcat(*response_buffer, "Content-Type: image/jpeg\r\n");
                    break;
                case 'g':
                    strcat(*response_buffer, "Content-Type: image/gif\r\n");
                    break;
                default:
                    strcat(*response_buffer, "Content-Type: text/html; charset=utf-8\r\n");
                    break;
                }
                break;
            }
        }

        // Content-Length
        strcat(*response_buffer, "Content-Length: 89\r\n");
        
        // Connection
        strcat(*response_buffer, "Connection: close\r\n");
        
        // 404 Error Message
        strcat(*response_buffer, "\r\n<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD><BODY><H1>404 Not Found</H1></BODY></HTML>");

        // printf("test");
        printf("%s", *response_buffer);
    }

    else  // Valid filename
    {
        
    }
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  //create socket
    if (sockfd < 0)
        error("ERROR opening socket");
    memset((char *) &serv_addr, 0, sizeof(serv_addr));  //reset memory
    //fill in address info
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    listen(sockfd,5);  //5 simultaneous connection at most
    
    //accept connections
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
    if (newsockfd < 0) {
        error("ERROR on accept");
      }
    
    int n;
    char buffer[256];
    char *response_buffer;
    response_buffer = (char*) malloc(1000); 
    int rb_len = 0;
    
    memset(buffer, 0, 256);  //reset memory
    
    //read client's message
    n = read(newsockfd,buffer,255);
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);

    //printSomething();
    
    //reply to client
    parse(buffer, &response_buffer);    // need & anywhere?
    char c = response_buffer[rb_len];
    while (c != NULL)
    {
        rb_len++;
        c = response_buffer[rb_len];
    }
    n = write(newsockfd, response_buffer, rb_len);
    if (n < 0) error("ERROR writing to socket");
    
    
    close(newsockfd);//close connection 
    close(sockfd);
    
    return 0; 
}