# S_CreateCharacterReply (3)

## 캐릭터 생성 응답

- 설명: 캐릭터 생성 요청에 대한 성공/실패 결과를 응답한다.
- 입력: 캐릭터 생성 결과 (success, detail)
- 처리 로직:
  - 캐릭터 생성 과정에서 발생한 결과를 클라이언트에 전달한다.
  - 실패 시 구체적인 에러 메시지를 detail 필드에 포함한다 (예: "등록된 이름입니다", "이름이 길거나 적합하지 않습니다").
  - 성공 시 success=true, detail="" 로 응답한다.
- 출력: `bool success, string detail`
- 데이터 구조: 없음 (단순 응답 패킷)