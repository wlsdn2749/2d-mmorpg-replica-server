# C_PartyList (62)

### **전체 공개 파티 목록 조회**

- 설명: 클라이언트가 서버에 생성된 모든 공개 파티의 목록을 요청한다.
- 입력: `C_PartyList`
  ```protobuf
  message C_PartyList {
      // 빈 메시지
  }
  ```
- 처리 로직:
  - `PartyManager::GetAllPublicParties()` 호출
  - `_parties` 맵의 모든 파티 순회
  - 각 파티의 `GetPartyInfo()` 메서드 호출
    - partyId, partyName, 멤버 목록, 현재/최대 인원, 리더 ID 수집
  - S_PartyList 패킷에 모든 파티 정보 추가
- 출력: `S_PartyList` (63번)
  - `repeated PartyInfo partyInfos`: 전체 파티 목록
- 데이터 구조:
  - `PartyManager::_parties`: unordered_map<int32, PartyRef>
  - `Party`: 각 파티의 정보

### 주의사항
- 현재 구현은 **모든 파티**를 반환 (파티명 유무 관계없이)
- 주석 처리된 코드: 파티명이 있는 파티만 필터링
  ```cpp
  // if (!party->GetPartyName().empty())
  ```
- 필요 시 파티명 필터링 활성화 가능

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 861-874)
- `GameServer/PartyManager.cpp` (Line 159-180)

### 클라이언트 처리
받은 파티 목록을 UI에 표시:
- 파티명
- 현재 인원 / 최대 인원 (예: 3/4)
- 리더 이름
- "가입 요청" 버튼
