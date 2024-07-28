import socket
import threading

class UDPClient:
    def __init__(self, ip="127.0.0.1", port=8080, buffer_size=1024):
        self.udp_ip = ip
        self.udp_port = port
        self.buffer_size = buffer_size
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.received_packets = set()
        self.total_packets = 0
        self.timeout_reached = False
        self.timeout = 0
        print("Digite 'sair' para encerrar a comunicação.")

    def start(self):
        while True:
            message = input("Digite 'iniciar' para solicitar pacotes ao servidor: ")

            if message.lower() == "sair":
                print("Encerrando o cliente.")
                self.sock.sendto("sair".encode(), (self.udp_ip, self.udp_port))
                break

            self.sock.sendto("Iniciar".encode(), (self.udp_ip, self.udp_port))

            self.receive_total_packets()

            self.receive_packets()

            self.timeout = int(self.timeout)//10

            missing_packets = self.get_missing_packets()

            while missing_packets:
                missing_packets = self.request_missing_packets(missing_packets)

            print("Todos os pacotes foram recebidos.")
        self.sock.close()

    def receive_total_packets(self):
        data, _ = self.sock.recvfrom(self.buffer_size)
        self.total_packets = int(data.decode())
        print(f"Total de pacotes a serem recebidos: {self.total_packets}")

    def receive_packets(self):
        self.timeout_reached = False
        timer = threading.Timer(self.timeout, self.set_timeout_reached)
        timer.start()

        while not self.timeout_reached:
            self.sock.settimeout(1.0)
            try:
                data, _ = self.sock.recvfrom(self.buffer_size)
                print(f"Recebido: {data.decode()}")
                self.received_packets.add(data.decode())
            except socket.timeout:
                continue

        timer.cancel()

    def set_timeout_reached(self):
        self.timeout_reached = True

    def get_missing_packets(self):
        missing_packets = [str(i+1) for i in range(self.total_packets) if f"Pacote {i+1}" not in self.received_packets]
        print(f"Pacotes faltando: {missing_packets}")
        return missing_packets

    def request_missing_packets(self, missing_packets):
        missing_request = ",".join(missing_packets)
        self.sock.sendto(missing_request.encode(), (self.udp_ip, self.udp_port))

        self.receive_packets()

        return self.get_missing_packets()

if __name__ == "__main__":
    client = UDPClient()
    client.start()
