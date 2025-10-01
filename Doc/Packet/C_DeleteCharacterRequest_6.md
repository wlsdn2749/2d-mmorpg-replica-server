# C_DeleteCharacterRequest (6)

### **캐릭터 삭제 요청**

- 설명: 클라이언트가 자신의 계정에서 특정 캐릭터를 삭제하는 요청을 전송한다. Soft Delete 방식으로 isActive=false 설정.
- 입력: `C_DeleteCharacterRequest`
  ```protobuf
  message C_DeleteCharacterRequest {
      int32 characterIndex = 1;  // 삭제할 캐릭터의 인덱스
  }
  ```
- 처리 로직:
  - 세션 JwtVerified 상태 검증
  - 계정 ID 조회 (JWT에서 추출)
  - 해당 계정의 캐릭터 목록 조회
  - characterIndex 범위 검증
  - DB Soft Delete 실행 (isActive=false 설정)
  - 캐릭터 리스트 재조회 및 캐시 갱신
- 출력: `S_DeleteCharacterReply` (7번)
  - `success`: 삭제 성공 여부
  - `errorMessage`: 실패 시 에러 메시지
- 데이터 구조:
  - `GameDB.Character` 테이블: isActive 컬럼 업데이트
  - 실제 데이터는 삭제되지 않음 (복구 가능)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
