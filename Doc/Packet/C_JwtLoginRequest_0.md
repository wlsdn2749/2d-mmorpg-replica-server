# C_JwtLoginRequest (0)

## JWT 인증

- 설명: 클라이언트가 보유한 JWT Access Token을 서버로 전송하여 인증을 요청한다.
- 입력: `C_JwtLoginRequest`
- 처리 로직:
  - 클라이언트에서 받은 accessToken을 JwtAuth::Verify()로 검증한다.
  - 검증 성공 시 Account 객체를 생성하고 userId를 저장한다.
  - AccountRepository::UpsertAccountAsync()를 호출하여 계정 정보를 DB에 저장한다.
  - 검증 결과(SUCCESS, INVALID_TOKEN, TOKEN_EXPIRED, SERVER_ERROR)를 클라이언트에 응답한다.
- 출력: `S_JwtLoginReply`
- 데이터 구조: 
  - Account 테이블 (계정 정보 저장)
  - JWT 검증을 위한 토큰 설정