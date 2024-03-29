# Function to generate a packet 
def get_packet(type, server_id=0, client_id=0, old_x=0, old_y=0, new_x=0, new_y=0, color=0, guess='', guess_length=0):
    packet = [0] * 4

    # Append universal headers 
    store_to_packet(packet, server_id, 0, 16)
    store_to_packet(packet, client_id, 16, 8)
    store_to_packet(packet, type, 24, 8)

    # Create draw update packet (TODO: type 0 can be removed later)
    if type == 1:
        for _ in range(12):
            packet.append(0)

        store_to_packet(packet, 1, 32, 16) # length = 1 as only 1 command
        store_to_packet(packet, old_x, 48, 16)
        store_to_packet(packet, old_y, 64, 16)
        store_to_packet(packet, new_x, 80, 16)
        store_to_packet(packet, new_y, 96, 16)
        store_to_packet(packet, color, 112, 16)
        
    # Create guess packet (TODO: type 6/7 can be removed later)
    elif type == 2 or type == 6 or type == 7:
        for _ in range(4 + guess_length):
            packet.append(0)

        store_to_packet(packet, guess_length, 32, 16)

        string_bytes = string_to_bytes(guess)
        for i in range(len(string_bytes)):
            store_to_packet(packet, string_bytes[i], 48 + i*8, 8)

    return bytes(packet)

# Function to parse a packet
def parse_packet(res):

    # Parse universal packet headers
    server_id = parse_bit_packet(res, 0, 16)
    client_id = parse_bit_packet(res, 16, 8)
    type = parse_bit_packet(res, 24, 8)

    print(f"Received packet with server {server_id}, client {client_id} and type {type}")
    commands = []
    word_len = 0
    word = ""

    # Parse canvas update packet
    if type == 0:
        command_len = parse_bit_packet(res, 32, 16) 
        for i in range(command_len):
            o_x = parse_bit_packet(res, 48 + i * 80, 16)
            o_y = parse_bit_packet(res, 64 + i * 80, 16)
            n_x = parse_bit_packet(res, 80 + i * 80, 16)
            n_y = parse_bit_packet(res, 96 + i * 80, 16)
            c = parse_bit_packet(res, 112 + i * 80, 16)
        
            commands.append({"old_x": o_x, "old_y": o_y, "new_x": n_x, "new_y": n_y, "color": c})
 
    # Get the word if the packet contains it 
    elif type == 4 or type == 5 or type == 6 or type == 7:
        # SET TYPE = DRAWER

        word_len = parse_bit_packet(res, 32, 16)
        word = bytes_to_string([parse_bit_packet(res, 48 + i*8, 8) for i in range(word_len)])

    data = {"server_id": server_id, 
            "client_id": client_id, 
            "type": type, 
            "commands": commands, 
            "word_len": word_len, 
            "word": word}

    return data

# Function to convert a string to bytes to be stored in a packet
def string_to_bytes(s):
    result = []

    for ch in s:
        ch = ord(ch)  # get char code
        st = []  # set up "stack"

        while ch:
            st.append(ch & 0xff)  # push byte to stack
            ch = ch >> 8  # shift value down by 1 byte

        # add stack contents to result
        # done because chars have "wrong" endianness
        result.extend(st[::-1])  # reverse the stack and add to result

    # return a list of bytes
    return result

# Function to store a value in a packet with a specific offset and length
def store_to_packet(packet, value, offset, length):
    # let us get the actual byte position of the offset
    last_bit_position = offset + length - 1
    number = bin(value)[2:]  # Convert to binary and remove '0b' prefix
    j = len(number) - 1

    for i in range(len(number)):
        byte_position = last_bit_position // 8
        bit_position = 7 - (last_bit_position % 8)

        if number[j] == "0":
            packet[byte_position] &= ~(1 << bit_position)
        else:
            packet[byte_position] |= 1 << bit_position

        j -= 1
        last_bit_position -= 1

# Function to get a value from a packet with a specific offset and length
def parse_bit_packet(packet, offset, length):
    number = 0
    for i in range(length):
        # let us get the actual byte position of the offset
        byte_position = (offset + i) // 8
        bit_position = 7 - ((offset + i) % 8)
        bit = (packet[byte_position] >> bit_position) % 2
        number = (number << 1) | bit
    return number

# Function to convert a list of bytes to a string
def bytes_to_string(array):
    result = ""
    for i in range(len(array)):
        result += chr(array[i])
    return result

# Function to print out a packet's contents
def print_packet_bit(packet):
    bit_string = ""

    for i in range(len(packet)):
        # To add leading zeros
        b = "00000000" + bin(packet[i])[2:]
        # To print 4 bytes per line
        if i > 0 and i % 4 == 0:
            bit_string += "\n"
        bit_string += " " + b[-8:]

    print(bit_string)