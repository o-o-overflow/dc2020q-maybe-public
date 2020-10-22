import sys

def load_bf(name):
    f = open(name,'rb')
    buf = f.read(1125)
    f.close()
    return buf

bloom = load_bf("vault_binsh.bf")
 
exec_ranges = [(1,22), (101,28), (253,10), (342,16), (270,4)]

exec_bytes = []
blacklist  = []

for start, size in exec_ranges:
    exec_bytes.extend(range(start,start+size))

print ("Executed bytes: %s"%exec_bytes)

for i in range(1125):
    if i not in exec_bytes:
        continue
    b = ord(bloom[i])

    if ((b & 2**0)==0): blacklist.append(i*8)
    if ((b & 2**1)==0): blacklist.append(i*8+1)
    if ((b & 2**2)==0): blacklist.append(i*8+2)
    if ((b & 2**3)==0): blacklist.append(i*8+3)
    if ((b & 2**4)==0): blacklist.append(i*8+4)
    if ((b & 2**5)==0): blacklist.append(i*8+5)
    if ((b & 2**6)==0): blacklist.append(i*8+6)
    if ((b & 2**7)==0): blacklist.append(i*8+7)

print ("Bit Blacklist: %s"%blacklist)

print ("%d bytes in the executed path"%len(exec_bytes))
print ("%d bit blacklisted"%len(blacklist))
