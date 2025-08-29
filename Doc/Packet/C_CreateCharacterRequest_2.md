# C_CreateCharacterRequest (2)

## 캐릭터 생성 요청

- 설명: 새로운 캐릭터 생성을 요청한다.
- 입력: `C_CreateCharacterRequest` (username, gender, region)
- 처리 로직:
  - 세션에서 userId를 가져와 유효성을 검증한다.
  - CharacterRepository::IsValidUsername()으로 캐릭터 이름의 유효성을 검증한다 (한글 2-6자, 중복 체크).
  - 성별(EGender)과 지역(ERegion) 검증을 수행한다.
  - 모든 검증 통과 시 CharacterRepository::CreateCharacterAsync()로 캐릭터를 생성한다.
- 출력: `S_CreateCharacterReply`
- 데이터 구조: 
  - Character 테이블 (캐릭터 기본 정보)
  - 캐릭터 이름 중복 검증 시스템