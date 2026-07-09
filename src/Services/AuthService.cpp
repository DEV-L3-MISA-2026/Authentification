#include <Services/AuthService.h>

#include <stdexcept>

#include <cpr/cpr.h>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>
#include <Repository/AuthRepository.h>

// Assumed dependency: this file uses jwt-cpp (https://github.com/Thalhammer/jwt-cpp)
// for signing/verifying and nlohmann::json for serializing the status map into
// a claim. Swap for your project's actual JWT library if different — only the
// encode()/decode() bodies below need to change.

using json = nlohmann::json;

namespace {
constexpr const char* kIssuer = "auth-service";
constexpr const char* kClaimSession = "sid";
constexpr const char* kClaimUserIds = "uids";
constexpr const char* kClaimStatuses = "statuses";

std::string factorKey(int userId) {
    return std::to_string(userId);
}
} // namespace

bool UserFactorStatus::isDone(AuthFactor factor) const {
    switch (factor) {
        case AuthFactor::Facial: return facial;
        case AuthFactor::Vocal: return vocal;
        case AuthFactor::Fingerprint: return fingerprint;
    }
    return false;
}

void UserFactorStatus::setDone(AuthFactor factor) {
    switch (factor) {
        case AuthFactor::Facial: facial = true; break;
        case AuthFactor::Vocal: vocal = true; break;
        case AuthFactor::Fingerprint: fingerprint = true; break;
    }
}

AuthService::AuthService( std::string jwtSecret,
                          std::string submitEndpointUrl)
    : jwtSecret_(std::move(jwtSecret)),
      submitEndpointUrl_(std::move(submitEndpointUrl)) {}

InitSessionResult AuthService::initSession(const std::string& payloadJson, const std::vector<int>& userIds) {
    // Persists the raw payload + user list to Postgres and returns a new
    // sessionId. This method needs to be added to AuthRepository:
    //   std::string createSession(const std::string& payloadJson, const std::vector<int>& userIds);
    auto authRepository = AuthRepository::getInstance();
    std::string sessionId = authRepository->createSession(payloadJson, userIds);

    AuthSession session;
    session.sessionId = sessionId;
    session.userIds = userIds;
    for (int userId : userIds) {
        session.statuses[userId] = UserFactorStatus{};
    }

    InitSessionResult result;
    result.sessionId = sessionId;
    result.token = encode(session);
    result.firstUserId = userIds.empty() ? -1 : userIds.front();
    return result;
}

std::string AuthService::encode(const AuthSession& session) const {
    json uids = json::array();
    for (int id : session.userIds) uids.push_back(id);

    json statuses = json::object();
    for (const auto& [userId, status] : session.statuses) {
        statuses[factorKey(userId)] = {
            {"facial", status.facial},
            {"vocal", status.vocal},
            {"fingerprint", status.fingerprint}
        };
    }

    auto token = jwt::create()
        .set_issuer(kIssuer)
        .set_type("JWS")
        .set_payload_claim(kClaimSession, jwt::claim(session.sessionId))
        .set_payload_claim(kClaimUserIds, jwt::claim(uids.dump()))
        .set_payload_claim(kClaimStatuses, jwt::claim(statuses.dump()))
        .sign(jwt::algorithm::hs256{jwtSecret_});

    return token;
}

AuthSession AuthService::decode(const std::string& token) const {
    jwt::decoded_jwt<jwt::traits::kazuho_picojson> decoded = jwt::decode(token);

    auto verifier = jwt::verify()
        .with_issuer(kIssuer)
        .allow_algorithm(jwt::algorithm::hs256{jwtSecret_});
    verifier.verify(decoded); // throws jwt::token_verification_exception on failure

    AuthSession session;
    session.sessionId = decoded.get_payload_claim(kClaimSession).as_string();

    json uids = json::parse(decoded.get_payload_claim(kClaimUserIds).as_string());
    for (const auto& id : uids) {
        session.userIds.push_back(id.get<int>());
    }

    json statuses = json::parse(decoded.get_payload_claim(kClaimStatuses).as_string());
    for (auto it = statuses.begin(); it != statuses.end(); ++it) {
        int userId = std::stoi(it.key());
        UserFactorStatus status;
        status.facial = it.value().value("facial", false);
        status.vocal = it.value().value("vocal", false);
        status.fingerprint = it.value().value("fingerprint", false);
        session.statuses[userId] = status;
    }

    return session;
}

bool AuthService::containsUser(const AuthSession& session, int userId) const {
    return session.statuses.find(userId) != session.statuses.end();
}

bool AuthService::isFactorDone(const AuthSession& session, int userId, AuthFactor factor) const {
    auto it = session.statuses.find(userId);
    if (it == session.statuses.end()) {
        throw std::runtime_error("userId not part of this session");
    }
    return it->second.isDone(factor);
}

void AuthService::markFactorDone(AuthSession& session, int userId, AuthFactor factor) {
    auto it = session.statuses.find(userId);
    if (it == session.statuses.end()) {
        throw std::runtime_error("userId not part of this session");
    }
    it->second.setDone(factor);
}

int AuthService::nextPendingUserId(const AuthSession& session) const {
    for (int userId : session.userIds) {
        auto it = session.statuses.find(userId);
        if (it == session.statuses.end() || !it->second.allComplete()) {
            return userId;
        }
    }
    return -1;
}


bool AuthService::finalizeSession(const std::string& sessionId) const {
    std::string payloadJson;
    try {
        payloadJson = authRepository_->getSessionPayload(sessionId);
    } catch (const std::exception&) {
        return false; // no such session, or DB error — nothing to submit
    }

    cpr::Response response = cpr::Post(
        cpr::Url{submitEndpointUrl_},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{payloadJson},
        cpr::Timeout{10000}
    );

    if (response.status_code < 200 || response.status_code >= 300) {
        return false;
    }

    authRepository_->markSessionSubmitted(sessionId);
    return true;
}
