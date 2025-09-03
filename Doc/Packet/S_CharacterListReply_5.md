# S_CharacterListReply (5)

## 캐릭터 목록 응답

- 설명: 요청한 계정의 캐릭터 목록을 응답한다.
- 입력: 캐릭터 데이터 목록 (CharacterSummaryInfo 배열)
- 처리 로직:
  - DB에서 조회한 캐릭터 정보를 CharacterSummaryInfo 형태로 변환한다.
  - 각 캐릭터의 이름, 레벨, 성별, 지역 정보를 포함한다.
  - repeated 필드를 사용하여 여러 캐릭터 정보를 한 번에 전송한다.
- 출력: `repeated CharacterSummaryInfo characters`
- 데이터 구조: 
  - Character 테이블에서 조회한 캐릭터 기본 정보