# S_SystemMessage (33)

### **시스템 메시지**

- 설명: 서버가 클라이언트에게 게임 내 시스템 메시지를 전송한다. 드롭 실패, 인벤토리 가득 참 등 다양한 상황에 사용.
- 입력: 서버 내부 이벤트 발생 시
- 처리 로직:
  - 특정 이벤트 발생 시 메시지 생성
  - 메시지 타입에 따라 클라이언트의 UI 표시 방식 다름
- 출력: `S_SystemMessage`
  ```protobuf
  message S_SystemMessage {
      string message = 1;
      EMessageType type = 2;
  }

  enum EMessageType {
      MESSAGE_INFO = 0;          // 일반 정보
      MESSAGE_WARNING = 1;       // 경고
      MESSAGE_ERROR = 2;         // 에러
      MESSAGE_DROP_FAILED = 3;   // 드롭 실패
  }
  ```
- 데이터 구조: 없음

### 사용 예시
- "인벤토리가 가득 찼습니다" (WARNING)
- "아이템을 획득했습니다" (INFO)
- "아이템 사용에 실패했습니다" (ERROR)
- "아이템을 버릴 수 없습니다" (DROP_FAILED)

### 관련 파일
- `GameServer/InventorySystem.cpp`
- `GameServer/ItemManager.cpp`
