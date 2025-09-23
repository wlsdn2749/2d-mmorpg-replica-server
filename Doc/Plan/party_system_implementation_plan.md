# 🎮 파티 시스템 구현 플랜 (개선된 버전)

## 📋 개요

기존 2D MMORPG 시스템에 **하이브리드 파티 시스템**을 추가하는 구현 계획입니다.
- **글로벌 파티 상태**: 룸 이동해도 파티 유지
- **같은 룸 경험치**: 같은 룸 파티원끼리만 경험치 분배
- **크로스 룸 동기화**: 다른 룸 파티원도 상태 표시
기존 코드베이스의 패턴과 아키텍처를 준수하며, 최대 4명까지 파티 구성이 가능합니다.

---

## 🎯 핵심 목표

- **하이브리드 아키텍처**: 글로벌 파티 + 룸별 경험치 분배 시스템
- **같은 룸 보상**: 같은 룸 파티원끼리만 경험치 분배 (처치자 110%, 파티원 각 10%)
- **크로스 룸 동기화**: 다른 룸에 있어도 파티 상태 표시 및 동기화
- **룸 이동 안정성**: 룸 이동 시에도 파티 상태 완벽 유지
- **확장 가능성**: 향후 길드/레이드 시스템 확장을 고려한 구조

---

## 🏗️ 1. 시스템 아키텍처 설계

### 1.1 핵심 클래스 구조 (기존 패턴 준수)

```cpp
// 타입 정의 (기존 패턴 따름)
using PartyRef = shared_ptr<Party>;

// 파티 핵심 클래스
class Party {
public:
    static constexpr int32 MAX_MEMBERS = 4;

    // 생성자
    explicit Party(int32 partyId, PlayerRef leader);

    // 멤버 관리
    bool AddMember(PlayerRef player);
    bool RemoveMember(PlayerRef player);
    bool IsFull() const { return _members.size() >= MAX_MEMBERS; }
    bool IsLeader(PlayerRef player) const;

    // 조회
    int32 GetPartyId() const { return _partyId; }
    PlayerRef GetLeader() const { return _leader; }
    const vector<PlayerRef>& GetMembers() const { return _members; }
    vector<PlayerRef> GetOnlineMembers() const;

private:
    int32 _partyId;
    PlayerRef _leader;
    vector<PlayerRef> _members;
};

// 파티 매니저 (기존 싱글톤 패턴 따름)
class PartyManager {
public:
    // 싱글톤 패턴 (RoomManager, ItemManager와 동일)
    static PartyManager& Instance() {
        static PartyManager instance;
        return instance;
    }

    // 초기화 (ItemManager 패턴 따름)
    bool Initialize();
    void Shutdown();

    // 파티 관리
    PartyRef CreateParty(PlayerRef leader);
    bool DisbandParty(int32 partyId);
    bool JoinParty(int32 partyId, PlayerRef player);
    bool LeaveParty(PlayerRef player);
    bool KickMember(int32 partyId, PlayerRef kicker, PlayerRef target);

    // 조회
    PartyRef FindParty(int32 partyId);
    PartyRef FindPlayerParty(PlayerRef player);
    int32 GetPlayerPartyId(PlayerRef player);

private:
    PartyManager() = default;
    ~PartyManager() = default;
    PartyManager(const PartyManager&) = delete;
    PartyManager& operator=(const PartyManager&) = delete;

    atomic<int32> _nextPartyId{1};
    unordered_map<int32, PartyRef> _parties;
    unordered_map<PlayerRef, int32> _playerToParty;
    shared_mutex _partyMutex;
    bool _initialized = false;
};

// Player 클래스 확장 (최소한의 수정)
class Player {
    // 기존 멤버들...
public:
    // 파티 관련 메서드
    inline int32 GetPartyId() const { return _partyId; }
    inline void SetPartyId(int32 partyId) { _partyId = partyId; }
    inline bool IsInParty() const { return _partyId != 0; }

private:
    int32 _partyId = 0;  // 파티 ID (0 = 파티 없음)
};
```

### 1.2 기존 시스템과의 통합점

- **Room 시스템**: 파티원 간 경험치 분배는 Room별로 처리
- **JobQueue**: 파티 관련 비동기 작업은 기존 JobQueue 활용
- **Player 시스템**: Player 클래스에 partyId 필드 추가
- **Protocol 시스템**: Protocol.proto에 파티 프로토콜 추가

---

## 🔧 2. 프로토콜 설계 (기존 패턴 준수)

### 2.1 새로운 MsgId 추가 (올바른 시작점)

```protobuf
enum MsgId {
    // 기존 프로토콜들... (47까지 사용 중)

    // 파티 시스템 프로토콜 (48~53)
    C_PARTY_INVITE_REQUEST = 48;
    S_PARTY_INVITE_REPLY = 49;
    S_PARTY_INVITE_NOTIFY = 50;
    C_PARTY_INVITE_RESPONSE = 51;
    C_PARTY_LEAVE = 52;
    S_BROADCAST_PARTY_UPDATE = 53;
}
```

### 2.2 간소화된 파티 메시지 구조 (기존 패턴 따름)

```protobuf
// (48) 클라 -> 서버: 파티 초대 요청
message C_PartyInviteRequest {
    int32 targetPid = 1;
}

// (49) 서버 -> 클라: 파티 초대 요청 결과
message S_PartyInviteReply {
    bool success = 1;
    string errorMessage = 2;
}

// (50) 서버 -> 클라: 파티 초대 알림 (초대받은 사람에게)
message S_PartyInviteNotify {
    int32 inviterPid = 1;
    int32 partyId = 2;
}

// (51) 클라 -> 서버: 파티 초대 응답 (수락/거절)
message C_PartyInviteResponse {
    int32 partyId = 1;
    bool accept = 2;
}

// (52) 클라 -> 서버: 파티 탈퇴/강퇴 요청
message C_PartyLeave {
    oneof action {
        bool selfLeave = 1;
        int32 targetPid = 2;
    }
}

// (53) 서버 -> 클라: 파티 상태 업데이트 (통합)
message S_BroadcastPartyUpdate {
    EPartyUpdateType updateType = 1;
    repeated PartyMemberInfoStatus members = 2;
}

// 통합된 파티 멤버 정보 구조
message PartyMemberInfoStatus {
    int32 playerId = 1;
    int32 hp = 2;
    int32 maxHp = 3;
    int32 level = 4;
    bool isLeader = 5;
}

enum EPartyUpdateType {
    PARTY_UPDATE_MEMBER_JOIN = 0;
    PARTY_UPDATE_MEMBER_LEAVE = 1;
    PARTY_UPDATE_STATUS = 2;
    PARTY_UPDATE_DISBANDED = 3;
}
```

---

## 🛠️ 3. 서버 구현 계획 (기존 패턴 준수)

### 3.1 Phase 1: 핵심 인프라 구축

#### 3.1.1 PartyService 분리 (시스템 분리)
```cpp
// GameServer/PartyService.h (경험치 분배 등 비즈니스 로직 처리)
class PartyService {
public:
    static PartyService& Instance() {
        static PartyService instance;
        return instance;
    }

    bool Initialize();
    void Shutdown();

    // 경험치 분배 (MonsterCombatSystem에서 호출)
    void DistributeExp(PlayerRef killer, int32 baseExp);

    // 파티 상태 동기화 (Room에서 주기적 호출)
    void UpdatePartyStatuses(const vector<PlayerRef>& roomPlayers);

private:
    PartyService() = default;
    ~PartyService() = default;
    PartyService(const PartyService&) = delete;
    PartyService& operator=(const PartyService&) = delete;

    bool _initialized = false;
};
```

#### 3.2.2 ClientPacketHandler 확장 (기존 패턴 준수)
```cpp
// Protocol/GenPackets.py가 자동 생성할 핸들러들
bool Handle_C_PartyInviteRequest(PacketSessionRef& session, Protocol::C_PartyInviteRequest& pkt);
bool Handle_C_PartyInviteResponse(PacketSessionRef& session, Protocol::C_PartyInviteResponse& pkt);
bool Handle_C_PartyLeave(PacketSessionRef& session, Protocol::C_PartyLeave& pkt);

// GameServer/ClientPacketHandler.cpp에 구현
bool Handle_C_PartyInviteRequest(PacketSessionRef& session, Protocol::C_PartyInviteRequest& pkt) {
    // 현재 플레이어의 Room JobQueue에서 처리
    auto gameSession = static_pointer_cast<GameSession>(session);
    if (!gameSession || !gameSession->_currentPlayer) return false;

    auto currentRoom = gameSession->_currentPlayer->GetCurrentRoom();
    if (!currentRoom) return false;

    // Room JobQueue를 통한 비동기 처리
    currentRoom->DoAsync(&Room::HandlePartyInvite,
                        gameSession->_currentPlayer, pkt.targetpid());
    return true;
}
```

### 3.2 Phase 2: Room 통합 (JobQueue 활용)

#### 3.2.1 Room 클래스 파티 기능 추가
```cpp
// GameServer/Room.h에 추가
class Room : public JobQueue {
    // 기존 멤버들...

public:
    // 파티 관련 비동기 처리 (JobQueue에서 실행)
    void HandlePartyInvite(PlayerRef inviter, int32 targetPid);
    void HandlePartyInviteResponse(PlayerRef player, int32 partyId, bool accept);
    void HandlePartyLeave(PlayerRef player, bool selfLeave, int32 targetPid);

    // 파티 상태 동기화 (OnTick에서 주기적 호출)
    void UpdatePartyStatuses();

private:
    // 파티 초대 대기 목록 (Room별로 관리)
    unordered_map<int32, chrono::steady_clock::time_point> _pendingInvites;
};
```

### 3.3 Phase 3: 경험치 분배 시스템 (PartyService 활용)

#### 3.3.1 MonsterCombatSystem 수정 (최소한의 변경)
```cpp
// GameServer/MonsterCombatSystem.cpp 수정
void MonsterCombatSystem::OnMonsterDeath(EntityId monsterId, PlayerRef killer) {
    // 기존 로직...
    int32 baseExp = GetMonsterExp(monsterId);

    // 파티 경험치 분배 (PartyService로 위임)
    if (killer->IsInParty()) {
        PartyService::Instance().DistributeExp(killer, baseExp);
    } else {
        // 기존 개인 경험치 지급
        killer->AddExp(baseExp);
    }
}
```

#### 3.3.2 PartyService 경험치 분배 구현
```cpp
// GameServer/PartyService.cpp
void PartyService::DistributeExp(PlayerRef killer, int32 baseExp) {
    auto party = PartyManager::Instance().FindPlayerParty(killer);
    if (!party) {
        // 파티가 없으면 개인 경험치
        killer->AddExp(baseExp);
        return;
    }

    auto onlineMembers = party->GetOnlineMembers();

    // 처치자: 110% 경험치
    killer->AddExp(static_cast<int32>(baseExp * 1.1f));

    // 나머지 파티원: 각각 10% 경험치
    for (auto& member : onlineMembers) {
        if (member != killer) {
            member->AddExp(static_cast<int32>(baseExp * 0.1f));
        }
    }
}
```

### 3.4 Phase 4: 상태 동기화 시스템 (RoomManager 패턴 활용)

#### 3.4.1 Room 틱 기반 파티 상태 업데이트
```cpp
// GameServer/Room.cpp OnTick 수정
void Room::OnTick() {
    // 기존 틱 로직...

    // 매 2초마다 파티 상태 업데이트
    if (_tick % 40 == 0) {  // 40 * 50ms = 2초
        UpdatePartyStatuses();
    }
}

void Room::UpdatePartyStatuses() {
    vector<PlayerRef> roomPlayers;
    for (auto& [playerId, player] : _players) {
        if (player->IsInParty()) {
            roomPlayers.push_back(player);
        }
    }

    if (!roomPlayers.empty()) {
        PartyService::Instance().UpdatePartyStatuses(roomPlayers);
    }
}
```

#### 3.4.2 PartyService 상태 동기화 구현
```cpp
// GameServer/PartyService.cpp
void PartyService::UpdatePartyStatuses(const vector<PlayerRef>& roomPlayers) {
    unordered_map<int32, vector<PlayerRef>> partiesInRoom;

    // 파티별로 그룹화
    for (auto& player : roomPlayers) {
        int32 partyId = player->GetPartyId();
        partiesInRoom[partyId].push_back(player);
    }

    // 각 파티별로 상태 업데이트 전송
    for (auto& [partyId, members] : partiesInRoom) {
        SendPartyStatusUpdate(partyId, members);
    }
}

void PartyService::SendPartyStatusUpdate(int32 partyId, const vector<PlayerRef>& members) {
    auto party = PartyManager::Instance().FindParty(partyId);
    if (!party) return;

    // S_BroadcastPartyUpdate 패킷 생성
    Protocol::S_BroadcastPartyUpdate pkt;
    pkt.set_updatetype(Protocol::PARTY_UPDATE_STATUS);

    // 파티 전체 멤버의 상태 정보 수집
    for (auto& member : party->GetOnlineMembers()) {
        auto* memberInfo = pkt.add_members();
        memberInfo->set_playerid(static_cast<int32>(member->playerId));
        memberInfo->set_hp(member->Hp());
        memberInfo->set_maxhp(member->MaxHp());
        memberInfo->set_level(member->Level());
        memberInfo->set_isleader(party->IsLeader(member));
    }

    // 파티원들에게 브로드캐스트
    BroadcastToPartyMembers(party, pkt);
}
```

---

## 🎨 4. 클라이언트 구현 계획 (기존 패턴 준수)

### 4.1 DummyClientCS 테스트 기능

#### 4.1.1 PacketManager 확장 (기존 패턴)
```csharp
// DummyClientCS/Packet/ClientPacketManager.cs에 추가
class ClientPacketManager {
    // 기존 핸들러들...

    public static void S_PartyInviteReplyHandler(PacketSession session, IMessage packet) {
        var pkt = packet as S_PartyInviteReply;
        if (!pkt.Success) {
            Console.WriteLine($"[파티] 초대 실패: {pkt.ErrorMessage}");
        }
    }

    public static void S_PartyInviteNotifyHandler(PacketSession session, IMessage packet) {
        var pkt = packet as S_PartyInviteNotify;
        Console.WriteLine($"[파티] PID {pkt.InviterPid}님의 파티 초대");

        // 자동 수락 (테스트용)
        C_PartyInviteResponse responsePkt = new C_PartyInviteResponse();
        responsePkt.PartyId = pkt.PartyId;
        responsePkt.Accept = true;
        session.Send(responsePkt);

        Console.WriteLine("[파티] 자동 수락됨");
    }

    public static void S_BroadcastPartyUpdateHandler(PacketSession session, IMessage packet) {
        var pkt = packet as S_BroadcastPartyUpdate;

        switch (pkt.UpdateType) {
            case EPartyUpdateType.PartyUpdateMemberJoin:
                Console.WriteLine($"[파티] 새 멤버 참가: {string.Join(", ", pkt.Members.Select(m => $"PID {m.PlayerId}"))}");
                break;
            case EPartyUpdateType.PartyUpdateMemberLeave:
                Console.WriteLine($"[파티] 멤버 퇴장. 남은 멤버: {string.Join(", ", pkt.Members.Select(m => $"PID {m.PlayerId}"))}");
                break;
            case EPartyUpdateType.PartyUpdateStatus:
                // 상태 업데이트는 너무 빈번하므로 필요시에만 출력
                break;
        }
    }
}
```

#### 4.1.2 DummyClient 파티 명령어 (기존 입력 패턴)
```csharp
// DummyClientCS/Program.cs 명령어 처리 추가
static void HandleCommand(string input, ServerSession session) {
    string[] tokens = input.Split(' ');
    string command = tokens[0].ToLower();

    switch (command) {
        // 기존 명령어들...

        case "/invite":
            if (tokens.Length < 2) {
                Console.WriteLine("사용법: /invite <playerid>");
                return;
            }
            if (int.TryParse(tokens[1], out int targetPid)) {
                SendPartyInvite(session, targetPid);
            } else {
                Console.WriteLine("올바른 플레이어 ID를 입력하세요.");
            }
            break;

        case "/accept":
            // 최근 초대를 자동으로 수락 (현재는 자동 수락 중)
            Console.WriteLine("파티 초대는 자동으로 수락됩니다.");
            break;

        case "/leave":
            SendPartyLeave(session, true, 0);
            break;

        case "/kick":
            if (tokens.Length < 2) {
                Console.WriteLine("사용법: /kick <playerid>");
                return;
            }
            if (int.TryParse(tokens[1], out int kickTargetPid)) {
                SendPartyLeave(session, false, kickTargetPid);
            } else {
                Console.WriteLine("올바른 플레이어 ID를 입력하세요.");
            }
            break;

        case "/party":
            Console.WriteLine("현재 파티 상태를 서버에서 확인하세요.");
            break;
    }
}

static void SendPartyInvite(ServerSession session, int targetPid) {
    C_PartyInviteRequest pkt = new C_PartyInviteRequest();
    pkt.TargetPid = targetPid;
    session.Send(pkt);
    Console.WriteLine($"[파티] PID {targetPid}님을 초대했습니다.");
}

static void SendPartyLeave(ServerSession session, bool selfLeave, int targetPid) {
    C_PartyLeave pkt = new C_PartyLeave();
    if (selfLeave) {
        pkt.SelfLeave = true;
        Console.WriteLine("[파티] 파티를 탈퇴했습니다.");
    } else {
        pkt.TargetPid = targetPid;
        Console.WriteLine($"[파티] PID {targetPid}님을 강퇴했습니다.");
    }
    session.Send(pkt);
}
```

### 4.2 Unity 클라이언트 UI (향후 확장)

#### 4.2.1 파티 UI 레이아웃
- **위치**: 화면 좌측 상단
- **구성요소**:
  - 파티장 표시 (왕관 아이콘)
  - 파티원 이름, 레벨, HP/MP 바
  - 미니맵에 파티원 위치 표시
  - 파티 관리 버튼 (초대, 탈퇴, 강퇴)

---

## 📊 5. 데이터 저장 정책 (메모리 기반)

### 5.1 영구 저장 없음 (세션 기반)

```cpp
// 파티는 메모리에서만 관리 (게임 세션 기반)
// - 서버 재시작 시 모든 파티 해체
// - DB 저장 없음으로 복잡성 최소화
// - 빠른 성능과 간단한 구조

class PartyManager {
    // 메모리 기반 저장소
    unordered_map<int32, PartyRef> _parties;
    unordered_map<PlayerRef, int32> _playerToParty;

    // 서버 종료 시 자동 정리
    void Shutdown() {
        _parties.clear();
        _playerToParty.clear();
        _initialized = false;
    }
};
```

### 5.2 Player 클래스 임시 필드만 추가

```cpp
// Player.h에 최소한의 수정
class Player {
    // 기존 멤버들...
private:
    int32 _partyId = 0;  // 런타임에만 유지되는 파티 ID

    // DB 저장 없음 - 메모리에서만 관리
    // SaveCharacterToDB()에서 _partyId는 제외
};
```

---

## 🧪 6. 테스트 시나리오

### 6.1 기본 파티 기능 테스트

1. **파티 생성 및 초대**
   - A가 B를 초대 → 파티 자동 생성
   - B가 수락 → 2인 파티 구성 확인
   - C, D 순차 초대 → 4인 파티 구성

2. **경험치 분배 테스트**
   - 4인 파티에서 A가 몬스터 처치
   - A: 110% 경험치 획득 확인
   - B, C, D: 각각 10% 경험치 획득 확인

3. **파티 관리 테스트**
   - 파티장이 특정 멤버 강퇴
   - 멤버 자발적 탈퇴
   - 네트워크 끊김 시 자동 탈퇴

### 6.2 엣지 케이스 테스트

1. **동시성 테스트**
   - 동시에 여러 파티 초대 요청
   - 파티 가득 찬 상태에서 초대 시도
   - 이미 파티에 속한 플레이어 초대 시도

2. **네트워크 안정성**
   - 파티원 중 일부 연결 끊김
   - 파티장 연결 끊김 시 처리
   - 룸 이동 중 파티 상태 유지

---

## 🚀 7. 구현 우선순위 (최적화된 단계)

### Phase 1: 프로토콜 및 핵심 클래스 (3-4일)
- [ ] Protocol.proto에 파티 메시지 추가 (C_PartyInviteRequest ~ S_BroadcastPartyUpdate)
- [ ] GenPackets.bat 실행하여 코드 생성
- [ ] Party, PartyManager, PartyService 클래스 기본 구조 구현
- [ ] Player 클래스에 _partyId 필드와 접근자 추가

### Phase 2: 기본 파티 기능 (3-4일)
- [ ] ClientPacketHandler에 파티 핸들러 추가
- [ ] Room에 파티 초대/응답 처리 로직 구현
- [ ] PartyManager 파티 생성/해체/가입/탈퇴 기능 완성
- [ ] DummyClient 테스트 명령어 추가 (/invite, /leave)

### Phase 3: 경험치 분배 및 상태 동기화 (2-3일)
- [ ] PartyService 경험치 분배 로직 구현
- [ ] MonsterCombatSystem에 파티 경험치 연동
- [ ] Room OnTick에 파티 상태 업데이트 추가
- [ ] 파티원 HP/위치 정보 주기적 브로드캐스트

### Phase 4: 테스트 및 안정화 (2-3일)
- [ ] DummyClient 다중 인스턴스 파티 테스트
- [ ] 동시성 이슈 및 메모리 누수 체크
- [ ] 에러 케이스 처리 (존재하지 않는 유저 초대, 이미 파티인 상태 등)
- [ ] 성능 최적화 및 로그 정리

**총 예상 기간: 10-14일**

---

## ⚡ 8. 성능 및 안정성 고려사항

### 8.1 메모리 관리 (기존 패턴 준수)
```cpp
// PlayerRef 활용으로 안전한 참조 관리
class Party {
    vector<PlayerRef> _members;  // shared_ptr 기반 안전한 참조

    // 자동 정리: 모든 멤버가 연결 해제되면 파티 해체
    bool HasOnlineMembers() const {
        return std::any_of(_members.begin(), _members.end(),
            [](const PlayerRef& player) { return player && player->IsOnline(); });
    }
};

// 메모리 누수 방지
class PartyManager {
    void CleanupEmptyParties() {
        auto it = _parties.begin();
        while (it != _parties.end()) {
            if (!it->second || !it->second->HasOnlineMembers()) {
                _playerToParty.erase(it->second->GetLeader());
                it = _parties.erase(it);
            } else {
                ++it;
            }
        }
    }
};
```

### 8.2 동시성 처리 (JobQueue 활용)
```cpp
// Room JobQueue 기반 순차 처리로 동시성 이슈 해결
class Room : public JobQueue {
    void HandlePartyInvite(PlayerRef inviter, const string& targetUsername) {
        // JobQueue에서 실행되므로 thread-safe
        // 별도 락 불필요
    }
};

// PartyManager는 조회만 담당, 수정은 Room에서
class PartyManager {
    // Read-only 메서드는 shared_lock 사용
    PartyRef FindParty(int32 partyId) const {
        shared_lock<shared_mutex> lock(_partyMutex);
        auto it = _parties.find(partyId);
        return (it != _parties.end()) ? it->second : nullptr;
    }

    // Write 메서드는 unique_lock 사용
    bool AddParty(PartyRef party) {
        unique_lock<shared_mutex> lock(_partyMutex);
        _parties[party->GetPartyId()] = party;
        return true;
    }
};
```

### 8.3 네트워크 최적화
- **업데이트 주기**: 2초마다 상태 업데이트 (과도한 트래픽 방지)
- **조건부 전송**: HP나 위치 변화가 있을 때만 업데이트
- **패킷 통합**: 여러 정보를 하나의 S_PartyUpdate로 전송

---

## 🔧 9. 구현 시 주의사항

### 9.1 기존 시스템과의 호환성
- **Player 저장**: SaveCharacterToDB()에서 _partyId 제외
- **Room 전환**: 룸 이동 시 파티 상태 유지
- **연결 해제**: OnDisconnected에서 파티 탈퇴 처리

### 9.2 에러 처리
```cpp
// 철저한 nullptr 체크
bool PartyService::InvitePlayer(PlayerRef inviter, const string& targetUsername) {
    if (!inviter) return false;

    auto target = FindPlayerByUsername(targetUsername);
    if (!target) {
        SendErrorMessage(inviter, "존재하지 않는 플레이어입니다.");
        return false;
    }

    if (target->IsInParty()) {
        SendErrorMessage(inviter, "이미 파티에 속한 플레이어입니다.");
        return false;
    }

    // ... 파티 초대 로직
}
```

### 9.3 테스트 시나리오
- **다중 클라이언트**: DummyClient 4개 인스턴스로 최대 파티 테스트
- **네트워크 끊김**: 파티장/멤버 연결 해제 시 처리
- **동시 초대**: 같은 플레이어에게 여러 초대 동시 발생

---

## 📈 10. 향후 확장 계획

### 10.1 단계적 확장 (기존 구조 활용)
```cpp
// 파티 타입 확장 가능한 구조
enum class EPartyType {
    HUNTING_PARTY = 0,    // 현재 구현
    RAID_PARTY = 1,       // 향후 확장
    GUILD_PARTY = 2,      // 길드 전용 파티
    QUEST_PARTY = 3       // 퀘스트 전용 파티
};

class Party {
    EPartyType _partyType = EPartyType::HUNTING_PARTY;
    // 타입별 설정 확장 가능
};
```

### 10.2 UI/UX 개선 (Unity 클라이언트)
- **실시간 파티 상태 UI**: 좌측 상단 파티 창
- **파티원 미니맵 표시**: 다른 Room에 있어도 위치 표시
- **파티 채팅**: 파티 전용 채팅 채널
- **빠른 초대**: 근처 플레이어 우클릭 초대

### 10.3 시스템 연계 확장
- **몬스터 난이도 조정**: 파티원 수에 따른 동적 난이도
- **파티 퀘스트**: 협력 퀘스트 시스템
- **아이템 분배**: 드랍 아이템 분배 시스템 (옵션)
- **길드 시스템**: 파티 → 길드 자연스러운 확장

---

## 📝 마무리

### ✅ 핵심 개선사항

1. **기존 코드 패턴 준수**: PlayerRef, Room JobQueue, 싱글톤 패턴 등 완전 준수
2. **타입 체계 정리**: EntityId → int32/PlayerRef로 의미적 명확성 확보
3. **프로토콜 최적화**: 6개 메시지 → 3개 통합 메시지로 단순화
4. **메모리 기반 설계**: DB 저장 없이 세션 기반으로 복잡성 최소화
5. **단계별 구현**: 10-14일 내 완성 가능한 현실적 계획

### 🎯 최종 목표

**"기존 시스템과 완벽하게 조화되는 간소하고 안정적인 파티 시스템"**

- 사용자 관점: 직관적이고 편리한 파티 구성
- 개발자 관점: 기존 코드 패턴과 일관성 있는 구조
- 성능 관점: 최소한의 오버헤드로 최대 효과
- 확장성 관점: 향후 길드/레이드 시스템으로 자연스러운 발전

이 플랜은 **실용성과 안정성**을 최우선으로 하며, 기존 2D MMORPG 시스템의 품질을 유지하면서도 의미 있는 협력 플레이 기능을 제공합니다.