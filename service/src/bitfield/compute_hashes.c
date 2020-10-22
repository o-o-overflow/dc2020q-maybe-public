#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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


int main(int argc, char* argv[]){ 
  char* buffer = argv[1];
  int len = 5;
  unsigned int x,y,z;
  /* register unsigned int a = murmurhash2(buffer, len, 0x9747b28c) % 9000; */
  /* register unsigned int b = murmurhash2(buffer, len, a) % 9000; */
  /* register unsigned int c = murmurhash2(buffer, len, b) % 9000; */
  register unsigned int a = murmurhash2(buffer, len, 0x9747b28c);
  register unsigned int b = murmurhash2(buffer, len, a);
  register unsigned int c = murmurhash2(buffer, len, b);
  x = a % 9000;
  y = b % 9000;
  z = c % 9000;
  
  printf("%d  %d  %d\n",x,y,z);
  
  return 0;
}


