import re

class ProtoParser:
    def __init__(self, start_id: int, recv_prefix: str, send_prefix: str):
        self.recv_pkt = []   # 수신 패킷 목록
        self.send_pkt = []   # 송신 패킷 목록
        self.total_pkt = []  # 모든 패킷 목록
        self.start_id = start_id
        self.id = start_id
        self.recv_prefix = recv_prefix.upper()
        self.send_prefix = send_prefix.upper()

    def parse_proto(self, path: str):
        in_msgid = False

        with open(path, 'r', encoding='utf-8') as f:
            for raw in f:
                line = raw.strip()

                # 주석/빈줄 스킵
                if not line or line.startswith("//"):
                    continue

                # MsgId 블록 시작
                if not in_msgid:
                    if re.match(r'^\s*enum\s+MsgId\b', line):
                        in_msgid = True
                    continue

                # MsgId 블록 종료
                if "}" in line:
                    break

                # 주석 제거
                line = line.split("//", 1)[0].strip()
                if not line or line == ";":
                    continue

                # 이름 추출
                names = line.strip().split(" =")
                if(len(names) == 0):
                    continue

                name = names[0]
                #pkt = Packet(name_pascal, pid)
                #self.total_pkt.append(pkt)

                if name.upper().startswith(self.recv_prefix):
                    words = name.split("_");

                    msg_name = "";
                    for word in words:
                        msg_name += word[0].upper() + word[1:].lower()

                    packet_name = f"{self.recv_prefix}{msg_name[1:]}"
                    pkt = Packet(packet_name, self.id)
                    self.recv_pkt.append(pkt)
                    self.total_pkt.append(pkt)
                    self.id += 1
                elif name.upper().startswith(self.send_prefix):
                    words = name.split("_");
                    msg_name = "";
                    for word in words:
                        msg_name += word[0].upper() + word[1:].lower()

                    packet_name = f"{self.send_prefix}{msg_name[1:]}"
                    pkt = Packet(packet_name, self.id)
                    self.send_pkt.append(pkt)
                    self.total_pkt.append(pkt)
                    self.id += 1
                
                

class Packet:
    def __init__(self, name: str, id: int):
        self.name = name
        self.id = id

    def __repr__(self):
        return f"Packet(name='{self.name}', id={self.id})"