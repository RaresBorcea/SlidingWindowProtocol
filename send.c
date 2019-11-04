#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"
#include "header.h"
#include "queue.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc, char** argv){
  init(HOST,PORT);
  msg m;
  my_pkt p;
  int in; // input file
  struct stat buf; // used to get file size
  int current_confirmed = 0; // current no. of confirmed messages
  int total_messages = -1; // total no. of messages

  // using '/' as delim as it cannot be used in names on Windows or Unix
  char delim[2] = "/";

  int active_timeout = 0; // starting point to receive with timeout
  int delay = atoi(argv[3]);

  // getting window size
  int bdp = atoi(argv[2]) * delay * 1000;
  int window_size = bdp / (1404 * 8);
  int seq = 0; // seq_number

  // opening input file
  in = open(argv[1], O_RDONLY);
  if(in < 0) {
    printf("[SENDER] Error when opening input file. Exiting");
    return -1;
  }

  // getting file_dimension
  fstat(in, &buf);
  int size = (int) buf.st_size;

  // sending file_name, file_dimension, window_size
  char aux_itoa[12]; // used to convert int to string
  memset(&p, 0, sizeof(p));
  p.type = '0'; // file&window details
  sprintf(p.payload, "%s", argv[1]);
  strcat(p.payload, delim);
  sprintf(aux_itoa, "%d", size);
  strcat(p.payload, aux_itoa);
  strcat(p.payload, delim);
  memset(aux_itoa, 0, sizeof(aux_itoa));
  sprintf(aux_itoa, "%d", window_size);
  strcat(p.payload, aux_itoa);
  p.seq = seq++;
  p.error_sum = check_error((unsigned char*)&p + 1, sizeof(char) + sizeof(int) + strlen(p.payload));
  memset(&m, 0, sizeof(m));
  m.len = 2 * sizeof(char) + sizeof(int) + strlen(p.payload);
  memcpy(m.payload, &p, m.len);

  Queue window = initQueue(m);
  printf("[SENDER] Sent message no. 0: file&window info\n");
  send_message(&m);

  // sending data from file to fill window
  int nr;
  memset(&p, 0, sizeof(p));
  while(seq < window_size && (nr = read(in, p.payload, MAX_LEN - 6)) > 0) {
  	p.type = '2'; // data
  	p.seq = seq;
  	p.error_sum = check_error((unsigned char*)&p + 1, sizeof(char) + sizeof(int) + nr);
  	memset(&m, 0, sizeof(m));
  	m.len = 2 * sizeof(char) + sizeof(int) + nr;
  	memcpy(m.payload, &p, m.len);

  	window = enqueue(window, m);
  	printf("[SENDER] Sent message no. %d: data\n", seq++);
  	send_message(&m);
  	memset(&p, 0, sizeof(p));
  }

  // send rest of file
  while(1) {
  	memset(&m, 0, sizeof(m));
  	int res = 0; 
    if(active_timeout) { 
      // used after reaching the recv with timeout step
      res = recv_message_timeout(&m, 2 * delay);
      if(res < 0) { 
        // timeout
        memset(&m, 0, sizeof(m));
        m = first(window);

        // resend first unconfirmed message from window
        printf("[SENDER] Timeout. Resent message no. %d\n", (*((my_pkt*)m.payload)).seq);
        send_message(&m);
      }

    } else { 

      // for the rest of messages
      res = recv_message(&m);
      if(res < 0) { 
        printf("[SENDER] Message receive error. Exiting.\n");
        return -1;
      }

    }

    if(res >= 0) {
			memset(&p, 0, sizeof(p));
    		p = *((my_pkt*)m.payload);
    		if(p.type == '3') { 
          // ack received
     			current_confirmed++;
      		printf("[SENDER] Received ack for message no. %d\n", p.seq);
     			if(current_confirmed == total_messages) { 
            // received all confirmations
      			break;
     			}

      		memset(&m, 0, sizeof(m));
        	m = first(window);
        	window = dequeueValue(window, p.seq);

          // if recv ack different than first message, resend first from window
        	if(p.seq != (*((my_pkt*)m.payload)).seq) { 
        		memset(&m, 0, sizeof(m));
        		m = first(window);
        		printf("[SENDER] Lost or Reordered packet. Resent message no. %d: data\n", 
                (*((my_pkt*)m.payload)).seq);
         		window = dequeue(window);
      			window = enqueue(window, m);
       			send_message(&m);
        	}

    			memset(&p, 0, sizeof(p));
     			if((nr = read(in, p.payload, MAX_LEN - 6)) > 0) {
      			p.type = '2'; // data
  					p.seq = seq;
  					p.error_sum = check_error((unsigned char*)&p + 1, 
                sizeof(char) + sizeof(int) + nr);
 					  memset(&m, 0, sizeof(m));
  					m.len = 2 * sizeof(char) + sizeof(int) + nr;
  					memcpy(m.payload, &p, m.len);

  					window = enqueue(window, m);
  					printf("[SENDER] Sent message no. %d: data\n", seq++);
 					  send_message(&m);
      		} else if(nr == 0 && total_messages == -1) { 
            // EOF reached
      			total_messages = seq;

            // start using recv_message_timeout for last window to be send
            active_timeout = 1; 
      		}

    		} else if(p.type == '4') { 
          // nak received
  			  memset(&m, 0, sizeof(m));
			    m = first(window);
			    printf("[SENDER] Received nak. Resent message no. %d\n", (*((my_pkt*)m.payload)).seq);
			    window = dequeue(window);
			    window = enqueue(window, m);
			    send_message(&m);
    		}
		}
  }

  printf("[SENDER] Sent entire file.\n");
  close(in);
  
  return 0;
}
