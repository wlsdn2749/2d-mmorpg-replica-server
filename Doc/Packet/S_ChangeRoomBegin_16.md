# S_ChangeRoomBegin (16)

## 룸 변경 시작 알림

- 설명: 플레이어가 다른 룸으로 이동 시작할 때 클라이언트에 알림을 보낸다.
- 입력: 룸 변경 정보 (transitionId, mapId)
- 처리 로직:
  - 서버에서 룸 변경이 시작될 때 클라이언트에 알림을 전송한다.
  - 클라이언트는 이 패킷을 받아 입력을 잠그고 로딩 UI를 표시한다.
  - transitionId로 룸 변경 세션을 식별한다.
  - mapId로 로드해야 할 맵 정보를 전달한다.
- 출력: `int32 transitionId, int32 mapId`
- 데이터 구조: 
  - Room 전환 시스템
  - 맵 데이터 로딩 시스템