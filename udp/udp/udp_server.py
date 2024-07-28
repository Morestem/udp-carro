import socket
import time

class UDPServer:
    def __init__(self, ip="0.0.0.0", port=8080, buffer_size=1024, total_packets=200):
        self.udp_ip = ip
        self.udp_port = port
        self.buffer_size = buffer_size
        self.total_packets = total_packets
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.udp_ip, self.udp_port))
        print(f"Servidor UDP ouvindo na porta {self.udp_port}")

    def listen(self):
        while True:
            data, addr = self.sock.recvfrom(self.buffer_size)
            print(f"Mensagem recebida de {addr}: {data.decode()}")
            self.send_total_packets(addr)
            self.send_packets(addr)

            while True:
                missing_request, addr = self.sock.recvfrom(self.buffer_size)
                missing_packets = missing_request.decode().split(',')
                if missing_packets[0] == 'sair':
                    print(f"Conexão com {addr} encerrada.")
                    break
                self.send_missing_packets(missing_packets, addr)

    def send_total_packets(self, addr):
        self.sock.sendto(str(self.total_packets).encode(), addr)

    def send_packets(self, addr):
        for i in range(self.total_packets):
            response = f"Pacote {i+1}"
            self.sock.sendto(response.encode(), addr)
            time.sleep(0.1)
        print(f"{self.total_packets} pacotes enviados.")

    def send_missing_packets(self, missing_packets, addr):
        for packet in missing_packets:
            response = f"Pacote {packet}"
            self.sock.sendto(response.encode(), addr)
        print(f"Pacotes solicitados enviados: {missing_packets}")

if __name__ == "__main__":
    server = UDPServer()
    server.listen()
