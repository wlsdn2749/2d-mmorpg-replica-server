# S_PartyCreateReply (57)

### **파티 생성 요청 결과**

- 설명: 서버가 공개 파티 생성 요청의 성공/실패 결과를 전송한다.
- 입력: `C_PartyCreateRequest` 처리 완료 후
- 처리 로직:
  - C_PartyCreateRequest 처리 결과에 따라 응답 생성
  - 성공 시: success=true, message="Party created"
  - 실패 시: success=false, message에 실패 사유
- 출력: `S_PartyCreateReply`
  ```protobuf
  message S_PartyCreateReply {
      bool success = 1;
      string message = 2;
  }
  ```
- 데이터 구조: 없음 (응답 패킷)

### 메시지 종류
- "Party created": 파티 생성 성공
- "Already in party": 이미 다른 파티에 속해있음
- "Failed to create party": 파티 생성 실패 (내부 에러)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 708-744)
