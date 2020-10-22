import sys

def load_bf(name):
    f = open(name,'rb')
    buf = f.read(1125)
    f.close()
    return buf

b1 = load_bf("vault.bf")
b2 = load_bf("vault_binsh.bf")
    
bit = 0
bytes = 0

changes = []

for i in range(1125):
    x = ord(b1[i])
    y = ord(b2[i])

    err = 0
    if ((x & 2**0) and (not (y & 2**0))): err=1
    if ((x & 2**1) and (not (y & 2**1))): err=1 
    if ((x & 2**2) and (not (y & 2**2))): err=1 
    if ((x & 2**3) and (not (y & 2**3))): err=1 
    if ((x & 2**4) and (not (y & 2**4))): err=1 
    if ((x & 2**5) and (not (y & 2**5))): err=1 
    if ((x & 2**6) and (not (y & 2**6))): err=1 
    if ((x & 2**7) and (not (y & 2**7))): err=1 

    if err:
        print ("ERROR, byte at offset %x:"%i)
        print (" needs to be %.2x - %s"%(y,bin(y)))
        print (" but is      %.2x - %s"%(x,bin(x)))
        # sys.exit(1)

    before = bit
    if ((not (x & 2**0)) and (y & 2**0)): bit+=1; changes.append(i*8)
    if ((not (x & 2**1)) and (y & 2**1)): bit+=1; changes.append(i*8+1)
    if ((not (x & 2**2)) and (y & 2**2)): bit+=1; changes.append(i*8+2) 
    if ((not (x & 2**3)) and (y & 2**3)): bit+=1; changes.append(i*8+3) 
    if ((not (x & 2**4)) and (y & 2**4)): bit+=1; changes.append(i*8+4) 
    if ((not (x & 2**5)) and (y & 2**5)): bit+=1; changes.append(i*8+5) 
    if ((not (x & 2**6)) and (y & 2**6)): bit+=1; changes.append(i*8+6) 
    if ((not (x & 2**7)) and (y & 2**7)): bit+=1; changes.append(i*8+7) 

    if bit > before:
        bytes += 1

print ("Changes: %s"%changes)
print ("%d bytes, %d bits to change"%(bytes,bit))    
