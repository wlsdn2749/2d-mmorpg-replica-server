# S_ChangeRoomCommit (18)

## 룸 변경 완료 확정

- 설명: 룸 변경이 완료되어 최종 스폰 좌표와 월드 상태를 확정한다.
- 입력: 룸 변경 완료 정보 (transitionId, snapshots)
- 처리 로직:
  - 클라이언트가 준비 완료를 알린 후 서버에서 최종 룸 전환을 확정한다.
  - 새로운 룸에서의 스폰 좌표를 결정한다.
  - 현재 룸에 있는 모든 플레이어의 정보를 snapshots로 전송한다.
  - 클라이언트는 이 패킷을 받아 새로운 룸에서 게임을 시작한다.
- 출력: `int32 transitionId, S_PlayerList snapshots`
- 데이터 구조: 
  - Room 전환 시스템
  - 플레이어 스냅샷 데이터