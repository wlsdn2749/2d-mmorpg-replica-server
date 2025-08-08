#pragma once

using namespace Protocol;

class JwtAuth {
public:
	static void Init(std::string secret) {
		_secret = std::move(secret);
	}

    static ELoginResult Verify(const std::string& token) {
        try {
            auto decoded = jwt::decode(token);

            // 서명/iss/aud 검증
            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{ _secret })
                .with_issuer("GameAuth")
                .with_audience("GameClient")
                .verify(decoded);

            // 만료(exp) 수동 체크 (시계 오차 5초 허용)
            using namespace std::chrono;
            try {
                auto exp = decoded.get_expires_at();
                if (exp <= system_clock::now() - seconds(5)) {
                    return ELoginResult::TOKEN_EXPIRED;
                }
            }
            catch (...) {
                // exp 클레임 없음/파싱 실패 → invalid 처리
                return ELoginResult::INVALID_TOKEN;
            }

            return ELoginResult::SUCCESS;
        }
        catch (const std::exception& e) {
            // 서명 불일치, iss/aud 불일치 등
            return ELoginResult::INVALID_TOKEN;
        }
        catch (...) {
            return ELoginResult::SERVER_ERROR;
        }
    }
private:
	static inline std::string _secret;
		
};