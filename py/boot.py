import serial, os, math
from time import sleep
from struct import pack, unpack

PORT = '/dev/ttyUSB0'
FILE = '../app/firmware.bin'

REQ_ENTER = [0xde, 0xad, 0xbe, 0xef]
ACK  = [0xaa, 0xbb]
NACK = [0xde, 0xad]
BLOCK_SIZE = 64

def crc8_update(data, crc):
    crc ^= data
    for i in xrange(8):
        if crc & 0x80 != 0:
            crc = (crc << 1) ^ 0x07
        else:
            crc <<= 1
    return crc & 0xFF

def port_open():
    req = bytearray(REQ_ENTER)
    chunks = os.path.getsize(FILE)
    chunks = int(math.ceil(float(chunks) / BLOCK_SIZE))
    print 'Need to send', chunks, 'chunks'
    req.append(chunks)
    crc = get_crc()
    print 'CRC = ', hex(crc)
    req.append(crc)
    req.append(crc)
    #crc = pack('<H', crc)
    #req.extend(crc)
    #print [hex(i) for i in req]

    ser = serial.Serial(PORT, 115200, timeout=1.0)
    ser.write(req)
    ser.flushOutput()
    return ser

def get_crc():
    crc = 0
    data = open(FILE, 'rb')
    with data as f:
        chunk = bytearray(f.read(BLOCK_SIZE))
        while chunk:
            chunk.extend([0xFF] * (BLOCK_SIZE - len(chunk)))
            #crc = crc_hqx(chunk, crc)
            for i in chunk:
                crc = crc8_update(i, crc)
            chunk = bytearray(f.read(BLOCK_SIZE))
    return crc

def bootloader_write():
    ser = port_open()
    data = open(FILE, 'rb')
    total = 0
    with data as f:
        chunk = bytearray(f.read(BLOCK_SIZE))
        while chunk:
            rx = ser.read(2)
            if len(rx) != 2:
                print 'Timeout'
                return
            total += len(chunk)
            print total
            chunk.extend([0xFF] * (BLOCK_SIZE - len(chunk)))
            ser.write(chunk)
            ser.flushOutput()
            chunk = bytearray(f.read(BLOCK_SIZE))
        ack = ser.read(2)
        print ' '.join(i.encode('hex') for i in ack)
        if ack == bytearray(NACK):
            print 'CRC mismatch'
        print 'Done'
    ser.close()

if __name__ == "__main__":
    bootloader_write()
