import socket

if __name__ == "__main__":
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect(("127.0.0.1", 25565))
    except TimeoutError as e:
        print("timeout")
        raise e
    
    while True:
        try:
            buf = client_socket.recv(1024)
            print(buf.decode())
            client_socket.send(buf)
        except KeyboardInterrupt:
            client_socket.close()
            break
    print("end")