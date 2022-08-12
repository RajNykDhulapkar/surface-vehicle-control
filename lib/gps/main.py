st = "PMTK220,1500"
i = 0
checksum = 0
while i < len(st):
    checksum ^= ord(st[i])
    i += 1
print("%02X" % checksum)
