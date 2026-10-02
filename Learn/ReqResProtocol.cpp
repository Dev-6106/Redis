#include <bits/stdc++.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
using namespace std;


// 1 -> Make servers process multiple requests from same client

void multipleReq(int fd){
    // accept
    struct sockaddr_in addr = {};
    socklen_t addrlen = sizeof(addr);
    int connfd = accept(fd,(struct sockaddr*)&addr,&addrlen);
    if(connfd < 0){
        return; // error
    }

    // serve one client multiple times
    while(true){
        int32_t err = one_request(connfd);
        if(err){
            break;
        }
    }
    close(connfd);
}


// But how do we know how many bytes to read from client?
// New protocol -> Client passes first lenght of the message in 4 bytes
// We read that much bytes

// rv = read(fd,&rbuf,n); does not garuantee read of n bytes but at most n bytes
// rv < 0 -> error;
// rv -> no of bytes read;


// static function -> can be linked only in this file
static int32_t read_full(int fd, char *buf, size_t n){
    while(n > 0){
        ssize_t rv = read(fd,buf,n);
        if(rv <= 0){
            return -1; // error
        }
        
        assert((size_t)rv <= n);
        // will generate error and break if condition not satisfied
        
        buf+=rv;
        n-=rv;
    }
    return 0;
}

static int32_t write_all(int fd, char *buf, size_t n){
    while(n > 0){
        ssize_t wr = write(fd,buf,n);
        if(wr <= 0){
            return -1; // error
        }
        
        assert((size_t)wr <= n);
        buf+=wr;
        n-=wr;
    }
}

// Creating one_request function
// Reads one message from cleint -> size + payload
// Writes something back

const size_t k_max_msg = 4096;

void msg(string s){
    cout << "[ERROR]" << s << endl;
}

static int32_t one_request(int connfd){
    // 4 bytes buffer
    char rbuf[4 + k_max_msg];
    errno = 0;

    int32_t err = read_full(connfd, rbuf, 4);
    if(err){
        msg(errno == 0 ? "EOF": "read() error");
        // errno is updated by read function itself
        return err;
    }

    uint32_t len = 0;
    memcpy(&len,rbuf,4);
    // copy 4 bytes from buffer address to len address

    if(len > k_max_msg){
        msg("too long");
        return -1;
    }

    ssize_t rv = read_full(connfd,rbuf+4,len);
    if(rv){
        msg("read() errir");
        return rv;
    }

    printf("Client says:  %.*s\n",len, &rbuf[4]);

    // Write back
    char reply[] = "Thank you!";
    len = (uint32_t) strlen(reply);

    char wbuf[4 + len];
    memcpy(wbuf,&len,4);
    memcpy(wbuf+4,reply,len);

    int wr = write_all(connfd,wbuf,4+len);
    if(wr){
        msg("write() error");
        return wr;
    }
}