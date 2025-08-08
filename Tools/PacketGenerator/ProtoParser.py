

class ProtoParser():
    def __init__(self, start_id, recv_prefix, send_prefix):
        self.recv_pkt = [] # 수신 패킷 목록
        self.send_pkt = [] # 송신 패킷 목록

        self.total_pkt = [] # 모든 패킷 목록
        self.start_id = start_id
        self.id = start_id
        self.recv_prefix = recv_prefix
        self.send_prefix = send_prefix

    def parse_proto(self, path):

        with open(path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
  
            for line in lines:
                if line.startswith('message') == False:
                    continue

                pkt_name = line.split()[1].upper() # 대문자
                if pkt_name.startswith(self.recv_prefix):
                    self.recv_pkt.append(Packet(pkt_name, self.id))
                elif pkt_name.startswith(self.send_prefix):
                    self.send_pkt.append(Packet(pkt_name, self.id))
                else:
                    continue

                self.total_pkt.append(Packet(pkt_name, self.id))
                self.id += 1



class Packet():
    def __init__(self, name, id):
        self.name = name
        self.id = id

        



