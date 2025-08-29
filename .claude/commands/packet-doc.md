# Analyze protocol files and generate comprehensive packet documentation by examining implementation code while strictly maintaining read-only access to source files.


You follow this structured pipeline for the packet-doc task:

1. Read and analyze the /Protocol/protocol.proto file to extract all message definitions.

2. You can find <packet_name> corresponding to <packet_number> and remember that.

3. For each packet found, check if /Doc/Packet/<packet_name>_<packet_number>.md already exists
   - If exists: Note that it needs updating
   - If not exists: Note that it needs creation

4. Present the user with a list of packets found and their status (new/update needed)

5. For each packet, create or update the documentation file using this template inside template tags and if you want use like inside of example tags:

<template>
**[기능명]**
* 설명: [패킷이 수행하는 기능에 대한 설명]
* 입력: `[입력 패킷명]`
* 처리 로직:
   * [서버에서 처리하는 로직 단계별로 설명]
   * [데이터 검증, 비즈니스 로직, 응답 생성 등]
* 출력: `[출력 패킷명 또는 결과]`
* 데이터 구조: [필요한 DB 테이블이나 Config 파일들]
</template>
<example>
### **Jwt 인증**

- 설명: 클라이언트가 서버에 소지한 Jwt를 보내 인증하는 요청을 보낸다.
- 입력: `C_JwtLoginRequest`
- 처리 로직:
    - 입력받은 accessToken을 검증한다.
    - 검증한 결과에 대해 `ELoginResult` 를 반환한다.
- 출력: `ELoginResult`
</example>

6. Follow these analysis guidelines:
   - C_ prefix: Client → Server request packets
   - S_ prefix: Server → Client response/broadcast packets
   - Infer functionality from packet names (Login, Attack, Move, etc.)
   - Consider game server context: "클라이언트 콘텐츠 → 데이터 + 룰 변환"
   - Suggest appropriate database tables and config files
   - Include security and validation considerations

7. Create/update each /Doc/Packet/<packet_name>_<packet_number>.md file

8. For existing files, analyze current content and only supplement missing sections

9. Present a summary of created/updated files

10. MANDATORY: Reference and analyze the following files for actual implementation details:
    - GameServer/ClientPacketHandler for server-side packet processing logic
    - DummyClientCS/ServerPacketHandler.cs for client-side packet response handling
    - Follow the code execution flow when necessary to analyze structure and flow
    - You may analyze git logs to understand the evolution and context of packet implementations
    - Investigate what actually happens and reflect this in the documentation
  

11. CRITICAL RESTRICTIONS - DO NOT VIOLATE THESE RULES:
    - ONLY modify files in /Doc/Packet/ folder - NEVER touch any other code files
    - DO NOT perform any data migrations on your own
    - DO NOT change any API names or parameters
    - ONLY create or update documentation files, never modify implementation code