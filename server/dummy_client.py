import socket

def main():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        client_socket.connect(("127.0.0.1", 25565))
    except TimeoutError as e:
        print("timeout")
        raise e
    
    try:
        while True:
            buf = client_socket.recv(1024)
            print(buf.decode())
            print(client_socket.send(buf))
    except KeyboardInterrupt:
        print("Closing connection")
        client_socket.close()

    print("end")

if __name__ == "__main__":
    main()