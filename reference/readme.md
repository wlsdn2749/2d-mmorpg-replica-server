PacketManager.cs
-> GenPackets.bat를 통해 생성하기를 원하는 자동화 cs파일입니다. Python Jinja2 Template을 사용합니다. 
-> 기존 루키스님의 패킷 핸들러에서 IPacket부분을 IMessage로 치환하여 작성했습니다.

ServerPacketManager.cs
-> GenPackets.bat을 통해 생성된 파일입니다. 기존의 Protocol.proto를 사용해 만드는 방식은 동일합니다.

ServerPacketHander.cs
-> 이 부분이 실제로 컨텐츠를 구현해야 하는 부분입니다. 빌드되어 ServerPacketManager가 바뀌면 이 부분이 비어있다고 오류뜹니다.
-> 이 부분 또한 C# 구조를 그대로 따라갑니다.