# S_PartyInviteReply (52)

### **파티 초대 요청 결과**

- 설명: 서버가 초대 요청자에게 초대 요청의 성공/실패 결과를 전송한다.
- 입력: `C_PartyInviteRequest` 처리 완료 후
- 처리 로직:
  - C_PartyInviteRequest 처리 중 대상 플레이어 검색 결과에 따라 응답 생성
  - 성공 시: success=true
  - 실패 시: success=false, errorMessage 설정
- 출력: `S_PartyInviteReply`
  ```protobuf
  message S_PartyInviteReply {
      bool success = 1;          // 초대 요청 성공 여부
      string errorMessage = 2;   // 실패 시 에러 메시지
  }
  ```
- 데이터 구조: 없음 (응답 패킷)

### 에러 메시지 종류
- "Player not found": 대상 플레이어를 찾을 수 없음
- "Player not in room": 대상 플레이어가 룸에 없음
- "Target already in party": 대상이 이미 다른 파티에 속해있음 (예상 에러)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 584-635)
