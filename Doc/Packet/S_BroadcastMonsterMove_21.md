# S_BroadcastMonsterMove (21)

## 몬스터 이동 브로드캐스트

- 설명: 몬스터가 이동했을 때 룸 내 모든 플레이어에게 브로드캐스트한다.
- 입력: 몬스터 이동 정보 (monsterId, x, y, dir)
- 처리 로직:
  - MonsterMovementSystem에서 몬스터가 이동할 때 호출된다.
  - 이동한 몬스터의 새로운 위치와 방향을 전송한다.
  - 룸 내 모든 플레이어에게 브로드캐스트되어 몬스터 위치를 동기화한다.
  - 클라이언트는 이 정보를 받아 몬스터의 위치를 업데이트한다.
- 출력: `int32 monsterId, int32 x, int32 y, EDirection dir`
- 데이터 구조: 
  - MonsterMovementSystem
  - Room 브로드캐스트 시스템