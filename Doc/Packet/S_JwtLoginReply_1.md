# S_JwtLoginReply (1)

## JWT 인증 응답

- 설명: JWT 토큰 검증 결과를 클라이언트에 응답한다.
- 입력: JWT 검증 결과 (`ELoginResult`)
- 처리 로직:
  - 서버에서 JWT 검증을 완료한 후 결과를 패킷으로 생성한다.
  - SUCCESS, INVALID_TOKEN, TOKEN_EXPIRED, SERVER_ERROR 중 하나의 결과를 반환한다.
- 출력: `ELoginResult`
- 데이터 구조: 없음 (단순 응답 패킷)