def checksumCalculator(str: str):
    curr_crc = 0x0101
    sum1 = curr_crc
    sum2 = (curr_crc >> 8)
    for i in range(0, len(str)):
        sum1 = (sum1 + ord(str[i])) % 255
        sum2 = (sum2 + sum1) % 255
    return (sum2 << 8) | sum1


def signMessage(msg: str):
    msg = msg.strip()
    checkSum = checksumCalculator(msg[1:])
    return f"{msg}*{checkSum}\r\n"
