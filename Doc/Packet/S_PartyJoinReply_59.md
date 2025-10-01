# S_PartyJoinReply (59)

### **파티 가입 요청/응답 결과**

- 설명: 서버가 파티 가입 요청 전송 결과 또는 리더의 수락/거절 결과를 전송한다.
- 입력:
  - `C_PartyJoinRequest` 처리 완료 후 (요청 전송 결과)
  - `C_PartyJoinResponse` 처리 완료 후 (수락/거절 결과)
- 처리 로직:
  - 요청 전송 시: 요청이 리더에게 전달되었는지 여부
  - 수락/거절 시: 리더의 응답에 따라 결과 메시지 생성
- 출력: `S_PartyJoinReply`
  ```protobuf
  message S_PartyJoinReply {
      bool success = 1;
      string message = 2;
  }
  ```
- 데이터 구조: 없음 (응답 패킷)

### 메시지 종류

#### 요청 전송 시
- "Request sent to leader": 리더에게 요청 전달 성공
- "Party not found": 파티가 존재하지 않음
- "Party is full": 파티 인원이 가득 참 (4/4)
- "Request already pending": 이미 대기 중인 요청이 있음

#### 수락/거절 시
- "Accepted": 리더가 가입 수락
- "Rejected": 리더가 가입 거절

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 746-810, 812-859)

### 클라이언트 처리
- 요청 전송 결과: "가입 요청을 보냈습니다" 메시지 표시
- 수락: 파티 UI 표시
- 거절: "파티 가입이 거절되었습니다" 메시지 표시
