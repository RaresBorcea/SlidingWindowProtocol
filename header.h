#define MAX_LEN     1400

// Define packet structure
typedef struct {
  unsigned char error_sum;
  // 0 - file&window info, 2 - data, 3 - ack, 4 - nak
  unsigned char type;
  int seq; // sequence number
  char payload[MAX_LEN-6];
} __attribute__((packed)) my_pkt;

// Used as buffer in receiver
typedef struct { 
  int len; // payload size
  my_pkt pkt;
} __attribute__((packed)) pkt_buff;

// Create checksum for a given string with XOR
unsigned char check_error (unsigned char* payload, int len) {
  unsigned char res = 0;
  int i;
  for(i = 0; i < len; i++) {
    res = res ^ payload[i];
  }
  return res;
}