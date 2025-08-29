# S_SpawnMonster (19)

## 몬스터 스폰 알림

- 설명: 새로운 몬스터가 스폰되었을 때 클라이언트에 알림을 보낸다.
- 입력: 몬스터 스폰 정보 (monsterId, monsterTypeId, x, y, dir)
- 처리 로직:
  - 서버의 MonsterSpawnerSystem에서 몬스터를 생성할 때 호출된다.
  - 유니크한 monsterId와 몬스터 타입 ID를 전송한다.
  - 스폰 위치(타일 좌표)와 초기 방향을 포함한다.
  - 클라이언트는 이 정보를 받아 해당 위치에 몬스터를 생성한다.
- 출력: `int32 monsterId, int32 monsterTypeId, int32 x, int32 y, EDirection dir`
- 데이터 구조: 
  - MonsterSpawnerSystem
  - 몬스터 타입 설정 데이터