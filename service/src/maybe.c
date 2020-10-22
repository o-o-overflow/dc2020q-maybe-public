#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#ifdef F_NAMES
#else
#define set_bit      f_03a7832ab2
#define bloom_add    f_9983929234
#define printBits    f_99ab00acd2
#define murmurhash2  f_08aaaabb23
#define test_bit     f_ffa83ac65d
#define bloom_check  f_03b87ac711
#define recv         f_0366ffff10
#define add_from_file f_0366fff211
#define bloom_init   f_9bdeab8871
#define error        f_39fff93dcc
#define bye_bye      f_cc0c9c0c9c
#define get_param    f_1982439911
#define debug        f_cccd637dcd
#define load_bf      f_101010293a
#define process_input f_daaaad0000
#define reset        f_090912ff21
#endif

struct bloom
{
  unsigned char lock;
  unsigned char hashes;
  unsigned short int bytes;
  unsigned short int bits;
  unsigned char  bf[1220];
  unsigned short int ones;
};


void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=0;i<size;i++)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
        if (i % 10==9) puts("");
        else printf(".");
    }
    puts("");
}

// This is cut-and-pasted from the internet - nothing interesting here.
unsigned int murmurhash2(const void * key, int len, const unsigned int seed)
{
// 'm' and 'r' are mixing constants generated offline.
// They're not really 'magic', they just happen to work well.

    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value
    unsigned int h = seed ^ len;

    // Mix 4 bytes at a time into the hash
    const unsigned char * data = (const unsigned char *)key;

    if (len==0) return 0;

    while(len >= 4)
    {
        unsigned int k = *(unsigned int *)data;

        k *= m;

        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array

    switch(len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

int test_bit(struct bloom * bloom, unsigned int x)
{
  unsigned char * buf = bloom->bf;
  unsigned int byte = x >> 3;
  unsigned char c = buf[byte];       
  unsigned int mask = 1 << (x % 8);

  if (c & mask) return 1;
  return 0;
}

void set_bit(struct bloom * bloom, unsigned int x) 
{
  unsigned char * buf = bloom->bf;
  unsigned int byte = x >> 3;
  unsigned char c = buf[byte];        
  unsigned int mask = 1 << (x % 8);

  if (c & mask) return;

  printf("Setting bit %d\n",x); 
  buf[byte] = c | mask;
  bloom->ones += 1;
}


int bloom_add(struct bloom * bloom, const void * buffer, int len)
{
  register unsigned int a = murmurhash2(buffer, len, 0x9747b28c);
  register unsigned int b = murmurhash2(buffer, len, a);
  register unsigned int c = murmurhash2(buffer, len, b);
  register unsigned int x;
  register unsigned int i;

  if (bloom->lock){
      puts("\n-locked-\n"); fflush(stdout);
      return 0;
  }

  /* printf("Processing '%s' (len %d)\n",buffer, len); */
  /* printf("Trying to add %d %d %d \n",a , b , c); */
  x = a % bloom->bits;
  set_bit(bloom, x);
  
  x = b % bloom->bits;
  set_bit(bloom, x);

  x = c % bloom->bits;
  /* x = stupidhash(buffer, len) % bloom->bits; */
  set_bit(bloom, x);

  return 1;
}

static int bloom_check(struct bloom * bloom, const void * buffer, int len)
{
  register unsigned int a = murmurhash2(buffer, len, 0x9747b28c);
  register unsigned int b = murmurhash2(buffer, len, a);
  register unsigned int c = murmurhash2(buffer, len, b);
  register unsigned int x;
  register unsigned int i;

  x = a % bloom->bits;
  if (!test_bit(bloom, x)) return 0;
  
  x = b % bloom->bits;
  if (!test_bit(bloom, x)) return 0;
  
  x = c % bloom->bits;
  if (!test_bit(bloom, x)) return 0;

  return 1;  
}


struct bloom* bloom_init(unsigned short int bits)
{

  struct bloom *bloom = malloc(sizeof(struct bloom));
  bloom->lock   = 1;
  bloom->hashes = 3;
  bloom->ones   = 0;
  bloom->bits   = bits;
  bloom->bytes  = 0x04c3;
  return bloom;
}


/* void bloom_print(struct bloom * bloom) */
/* { */
/*   printf("bloom at %p\n", (void *)bloom); */
/*   printf(" bitfield at %p\n", (void *)bloom->bf); */
/*   printf(" ->bits = %d\n", bloom->bits); */
/*   printf(" ->bytes = %d\n", bloom->bytes); */
/*   printf(" ->set bits = %d\n", bloom->ones); */
/*   printBits(bloom->bytes, bloom->bf); */
/* } */


void add_from_file(char* filename, struct bloom *bloom){
  FILE* filePointer;
  int bufferLength;
  char buffer[82];
  int count;
  bufferLength = 80;
  buffer[bufferLength+1]='\0';
  count = 0;

  filePointer = fopen(filename, "r");
  while(fgets(buffer, bufferLength, filePointer)) {
    count += 1;
    bloom_add(bloom, buffer, strlen(buffer)-1);
    if (count > 100) break;
    /* printf("Adding '%s' (%ld)\n", buffer, strlen(buffer)); */
  }
  fclose(filePointer);
}


int recv(char* buf, int n){
    int count = 0;
    while (count < n){
        if (read(0, &buf[count], 1)==0)
           return count;
        count++;
    }
    return count;
}

void error(int code){
    printf("%d - ",code); fflush(stdout);
    write(1, "ERROR\n", 6);
    fflush(stdout);
    exit(1);
}

// this is the way you do inline assembly in watcom
// https://tuttlem.github.io/2015/10/04/inline-assembly-with-watcom.html
void bug();
#pragma aux bug= "push edi";

void bye_bye(struct bloom* bloom, int* garbage){
    bug();
    puts("That's all for now\nNo more tries\n");
}

void get_param(unsigned char *buf, unsigned char *cmdsize){
    unsigned char size;
    size = 0;
    if (recv(&size, 1)==0) {
        error(1);
    }
    if (size < 1) {
        error(2);
    }
    if (size > 128) {
        error(2);
    }
    if (recv(buf, size)!=size) 
        error(5);
    
    buf[size]='\0';
    *cmdsize = size;

    /* printf("received parameter of len %u\n",*cmdsize); fflush(stdout); */
}

void debug(struct bloom *bloom, unsigned char* param, int param_size){
    signed char c;
    if (param_size != 1){
        error(3);
    }
    c = param[0];
    if (c>1){
        puts("Invalid\n");
        error(7);
    }
    if ((c==1) & (bloom->lock==0)){
        puts("You cannot test for the flag after you modified the data.\n");
        return;
    }

    bloom->lock = c;
    if (bloom->lock==0){
        puts("\n-Unlocked-\n");
    }
}
    
unsigned int load_bf(const char* filename, int size, unsigned char* buffer){
    FILE *fileptr;
    size_t result;
    fileptr = fopen(filename, "rb");
    if (fileptr == 0) error(10);
    result = fread(buffer, size, 1, fileptr);
    /* printf("Loaded %d bytes\n", result); */
    fclose(fileptr);
    return result;
}

unsigned int reset(struct bloom *bloom){
    int read, i;
    bloom->lock    = 1;
    /* bloom->lock    = 0xeb */
    bloom->hashes  = 3;
    bloom->ones    = 0;
    read = load_bf("./vault.bf", bloom->bytes, &bloom->bf);

    /* if (read < bloom->bits/8) error(); */
    for(i=0; i<read; i++){
        if (bloom->bf[i] & 1) bloom->ones+=1;
        if (bloom->bf[i] & 2) bloom->ones+=1;
        if (bloom->bf[i] & 4) bloom->ones+=1;
        if (bloom->bf[i] & 8) bloom->ones+=1;
        if (bloom->bf[i] & 16) bloom->ones+=1;
        if (bloom->bf[i] & 32) bloom->ones+=1;
        if (bloom->bf[i] & 64) bloom->ones+=1;
        if (bloom->bf[i] & 128) bloom->ones+=1;
    }
    return 0;
}

void process_input(struct bloom *bloom, unsigned int ncmd)
{
    unsigned char cmd[10];
    unsigned char param[129];
    unsigned char param_size;
    unsigned int n, i;

    printf("[%.2d/42] > ",ncmd+1);
    fflush(stdout);

    n = recv(&cmd, 3);
    if (n!=3) error(4);

    cmd[3]='\0';
    fflush(stdout);
    
    if (cmd[0]=='F' && cmd[1]=='L' && cmd[2]=='G') {
        get_param(&param, &param_size);
        if (bloom_check(bloom, param, param_size)){
            puts("\n-Maybe-\n"); fflush(stdout);
        }
        else{
            puts("\n-Definitely Not-\n"); fflush(stdout);
        }
    }
    else if (cmd[0]=='D' && cmd[1]=='B' && cmd[2]=='G') {
        get_param(&param, &param_size);
        debug(bloom, param, param_size);
    }
    else if (cmd[0]=='R' && cmd[1]=='L' && cmd[2]=='D') {
        reset(bloom);
        puts("\n-Brand New-\n"); fflush(stdout);
    }
    else if (cmd[0]=='A' && cmd[1]=='D' && cmd[2]=='D') {
        get_param(&param, &param_size);
        if (bloom_add(bloom, param, param_size))
            puts("\n-Updated-\n"); fflush(stdout);
    }
    else if (cmd[0]=='B' && cmd[1]=='Y' && cmd[2]=='E') {
        puts("\nSee you soon\n");
        fflush(stdout);
        exit(0);
    }
    else{
        error(6);
    }
    for(i=0;i<128;i++) param[i]='\x00';
}


// We use pragma directives to set up custom colling conventions for each 
// function
// https://users.pja.edu.pl/~jms/qnx/help/watcom/compiler-tools/pragma32.html
#pragma aux bloom_add parm [ESI] [EDX] [EAX];
#pragma aux bloom_add value [ESI];

#pragma aux bloom_check parm [ESI] [EBX];
#pragma aux bloom_check value [EDI];

#pragma aux process_input parm [EBX] [ECX];
#pragma aux process_input value [EDX];

#pragma aux murmurhash2 parm [EAX] [EDX];
#pragma aux murmurhash2 value [ECX];

#pragma aux set_bit parm [ECX];

#pragma aux test_bit parm [EDI] [EDX];
#pragma aux test_bit value [EBX];

#pragma aux bye_bye parm [EDI] [EBX];

#pragma aux bloom_init parm [EDX];
#pragma aux bloom_init value [EDI];

#pragma aux reset parm [EAX];
#pragma aux reset value [ESI];

#pragma aux load_bf parm [ESI] [EAX];
#pragma aux load_bf value [EBX];

#pragma aux debug parm [ECX] [ESI] [EAX];

#pragma aux get_param parm [ECX] [EDX];

#pragma aux error parm [ECX] [ESI] [EAX];

#pragma aux recv parm [EBX] [ECX];
#pragma aux recv value [EDI];

#pragma code_seg ( "dontknow" );

int main(int argc, char* argv[]){ 
    int cmd_count = 0;
    struct bloom *bloom = bloom_init(9000);
    reset(bloom);
    /* bloom_print(bloom); */
    
    /* bye_bye(bloom); */
    write(1,"|--Welcome--|  |-v1.1-|\n",24);
    fflush(stdout);

    while (cmd_count < 42){
        process_input(bloom, cmd_count);
        cmd_count += 1;
    }

    bye_bye(bloom, &cmd_count);
    if (bloom) free(bloom);
    return 0;
}

