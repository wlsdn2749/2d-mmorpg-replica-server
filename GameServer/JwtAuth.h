#pragma once

using namespace Protocol;

struct VerifyResult {
    ELoginResult    result;
    int32           userId;
};
class JwtAuth {
public:
	static void Init(std::string secret) {
		_secret = std::move(secret);
	}

    static pair<ELoginResult, int32> Verify(const std::string& token) {
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
                    return { ELoginResult::TOKEN_EXPIRED, -1 };
                }
            }
            catch (...) {
                // exp 클레임 없음/파싱 실패 → invalid 처리
                return { ELoginResult::INVALID_TOKEN, -1 };
            }

            int32 userId = 0;
            try {
                auto subStr = decoded.get_payload_claim("sub").as_string();
                userId = std::stoi(subStr);
            }
            catch (...) {
                return { ELoginResult::INVALID_TOKEN, 0 };
            }

            return { ELoginResult::SUCCESS, userId };
        }

        catch (const exception& e)
        {
            // iss, aud 파싱 실패
            return { ELoginResult::INVALID_TOKEN, -1};
        }
        catch (...) {
            return { ELoginResult::SERVER_ERROR, -1 };
        }
    }
private:
	static inline std::string _secret;
		
};