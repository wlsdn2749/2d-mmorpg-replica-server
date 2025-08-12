### 폴더 구조

```
root/
├── 📂 AuthServer/              # 인증 서버 (.Net 9.0)
├── 📂 DummyClientCS/           # gRPC + TCP 통신 테스트를 위한 DummyClient
├── 📂 Common/                  # 자동화 도구의 위치
├── 📂 GameServer/              # TCP GameServer
├── 📂 DummyClientCS/           # gRPC를 테스트를 위한 DummyClient
├── 📂 IocpCore/                # Iocp 핵심 Core입니다. 건드릴일 거의 없음.
└── 📂 third_party/             # 빌드에 필요한 third_party입니다
```

### 빌드
설정 파일을 Github에 업로드 했습니다.

크게 2가지 환경 종속적인 부분이 있습니다.
1. DB 
   
   DB의 경우 로컬 my-sql의 경로, driver등 설치되어있는 경로를 정확히 입력해야하며, 이를 틀릴 시 빌드가 되지 않을 수 있습니다.

   또한 빌드 전에 SQL에 직접 접근해 GameDb와 AuthDb를 데이터베이스를 생성해야하고 빌드 시 GameServer의 경우 GameDB.xml을 통해 자동으로 ORM이 생성되지만, AuthServer의 경우 없습니다. 하여
   관련 DB Create T-SQL이 노션에 업데이트 되어있어 그것을 사용해야합니다.


2. IP
   
   저희 서버는 개발할때 0.0.0.0:Port로 열긴 하지만, 혹시라도 IP종속적으로 개발되어있을 수 있으므로 이 점 참고해주시면 감사하겠습니다.
