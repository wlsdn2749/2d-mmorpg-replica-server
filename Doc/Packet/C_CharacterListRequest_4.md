# C_CharacterListRequest (4)

## 캐릭터 목록 요청

- 설명: 현재 계정이 보유한 캐릭터 목록을 요청한다.
- 입력: `C_CharacterListRequest` (빈 메시지)
- 처리 로직:
  - 세션에서 userId를 가져온다.
  - CharacterRepository::GetCharactersByUserAsync()로 해당 사용자의 모든 캐릭터를 조회한다.
  - 각 캐릭터를 Player 객체로 변환하여 세션에 저장한다.
  - 캐릭터 정보를 CharacterSummaryInfo로 변환하여 응답 패킷에 담는다.
- 출력: `S_CharacterListReply`
- 데이터 구조: 
  - Character 테이블 (캐릭터 기본 정보)
  - Player 객체 (세션 내 캐릭터 관리)