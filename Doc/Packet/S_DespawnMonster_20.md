# S_DespawnMonster (20)

## 몬스터 디스폰 알림

- 설명: 몬스터가 제거될 때 클라이언트에 알림을 보낸다.
- 입력: 몬스터 제거 정보 (monsterId, reason)
- 처리 로직:
  - 몬스터가 사망하거나 기타 이유로 제거될 때 호출된다.
  - 디스폰 이유(EDespawnReason)를 함께 전송한다 (KILLED 등).
  - 클라이언트는 이 패킷을 받아 해당 몬스터를 화면에서 제거한다.
- 출력: `int32 monsterId, EDespawnReason reason`
- 데이터 구조: 
  - MonsterContainer 관리 시스템
  - EDespawnReason 열거형