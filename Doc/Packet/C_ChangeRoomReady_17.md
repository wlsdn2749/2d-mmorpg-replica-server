# C_ChangeRoomReady (17)

## 룸 변경 준비 완료 알림

- 설명: 클라이언트가 새로운 룸 로딩을 완료했음을 서버에 알린다.
- 입력: `C_ChangeRoomReady` (transitionId)
- 처리 로직:
  - S_ChangeRoomBegin을 받은 클라이언트가 리소스와 타일맵 로딩을 완료한 후 전송한다.
  - 서버는 이 패킷을 받아 플레이어가 준비되었음을 확인한다.
  - Room::ChangeRoomReady를 비동기로 호출하여 룸 전환을 진행한다.
  - 모든 준비가 완료되면 S_ChangeRoomCommit으로 최종 확정된다.
- 출력: `S_ChangeRoomCommit`
- 데이터 구조: 
  - Room 전환 시스템
  - 클라이언트 리소스 로딩 상태