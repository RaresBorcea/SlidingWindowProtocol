#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"
#include "header.h"

#define HOST "127.0.0.1"
#define PORT 10001
#define BUFLEN 1000

int main(int argc, char** argv) {
  msg m;
  my_pkt p;
  init(HOST,PORT);
  int out; // output file
  char tok_aux[200]; // used for strtok processing
  char delim[2] = "/"; // delim for strtok
  unsigned char check;
  int current_print = 0; // current no. of printed messages
  int size = -1; // file size
  pkt_buff buffer[BUFLEN];
  memset(buffer, 0, sizeof(buffer));
  int min = 0, max = 0, n = 0; // used to check for consecutive elements in buffer
  int is_first_print = 1; // skip first 2 packets for first print
  int window_size = 0; // size of sender window
  int total_len = 0; // size for current no. of messages

  while(1) {
    memset(&m, 0, sizeof(m));
    int res = recv_message(&m);
    if(res < 0) {
      printf("[RECEIVER] Receive error. Exiting.\n");
      return -1;
    }

    memset(&p, 0, sizeof(p));
    p = *((my_pkt*)m.payload);
    check = check_error((unsigned char*)&p + 1, m.len - 1);

    if(p.error_sum != check) { 
      // corrupt packet
      memset(&p, 0, sizeof(p));
      p.type = '4'; //nak
      memset(&m, 0, sizeof(m));
      m.len = 2 * sizeof(char) + sizeof(int) + strlen(p.payload);
      memcpy(m.payload, &p, m.len);

      printf("[RECEIVER] Wrong packet. Sent nak.\n");
      send_message(&m);

    } else { 

      // packet content is fine
      if((p.seq - current_print) >= 0 && buffer[p.seq - current_print].len == 0) {
        // if this is the first time a message was receieved, save it in buffer
        buffer[p.seq - current_print].pkt = p;
        buffer[p.seq - current_print].len = m.len - 2 * sizeof(char) - sizeof(int);
        n++;

        if(p.seq > max) {
          max = p.seq;
        }

        if(p.type == '0') { 
          // file&window info
          memcpy(tok_aux, p.payload, strlen(p.payload) + 1);
          char * name = strtok(tok_aux, delim);
          size = atoi(strtok(NULL, delim));
          window_size = atoi(strtok(NULL, delim));

          char * file_name = (char *) malloc(1 + strlen("recv_") + strlen(name));
          strcpy(file_name, "recv_");
          strcat(file_name, name);
          out = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          printf("[RECEIVER] Received file_name: %s.\n", file_name);

          if(out < 0) {
            printf("[RECEIVER] Error opening output file. Exiting.\n");
            return -1;
          }

          printf("[RECEIVER] Received file_size: %d bytes.\n", size);
          printf("[RECEIVER] Received window_size: %d messages.\n", window_size);
        } else {
          total_len += buffer[p.seq - current_print].len;
        }

        if(max - min == n - 1) { 
          // check for continuous sequence
          if(BUFLEN - n < window_size || total_len == size) { 
            // buff cannot fit more or EOF
            int pos; 
            int printing = 0; // 1 for printing to output_file
                              // 0 otherwise

            if(is_first_print == 1 && n >= 2) { 
              // print after receiving file name and size
              pos = 1; // do not print file&window info
              printing = 1;
              is_first_print = 0;
            } else if(is_first_print == 0) {
              pos = 0; // otherwise, print buffer from beginning
              printing = 1;
            }

            if(printing == 1) {
              while(pos < n) {
                write(out, buffer[pos].pkt.payload, buffer[pos].len);
                pos++; 
              }
              current_print += n;

              // first message from next sequence
              min = buffer[n - 1].pkt.seq + 1; 

              max = 0;
              n = 0;
              memset(buffer, 0, sizeof(buffer));
            }
          }  
        }

        int retain_seq = p.seq; // send ack for same seq no.

        memset(&p, 0, sizeof(p));
        p.type = '3'; // ack
        p.seq = retain_seq;
        memset(&m, 0, sizeof(m));
        m.len = 2 * sizeof(char) + sizeof(int) + strlen(p.payload);
        memcpy(m.payload, &p, m.len);

        printf("[RECEIVER] Sent ack for message no. %d.\n", retain_seq);
        send_message(&m);

        if(total_len == size) { 
          // the entire file was printed
          break;
        }
      }
    }
  }

  printf("[RECEIVER] Received entire file.\n");
  close(out);

  return 0;
}