#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

class AuthRepository;

// Order matters: this is also the order the frontend is expected to call the
// three verify endpoints in (facial -> vocal -> fingerprint).
enum class AuthFactor {
    Facial,
    Vocal,
    Fingerprint
};

struct UserFactorStatus {
    bool facial = false;
    bool vocal = false;
    bool fingerprint = false;

    bool isDone(AuthFactor factor) const;
    void setDone(AuthFactor factor);
    bool allComplete() const { return facial && vocal && fingerprint; }
};

// Decoded, in-memory view of a session JWT's payload.
struct AuthSession {
    std::string sessionId;
    std::vector<int> userIds;                  // ordered list = iteration order
    std::map<int, UserFactorStatus> statuses;   // per-user factor completion
};

struct InitSessionResult {
    std::string sessionId;
    std::string token;
    int firstUserId = -1;   // -1 if userIds was empty
};

class AuthService {
public:
    AuthService(std::string jwtSecret,
                std::string submitEndpointUrl);

    // POST /api/auth : persists the payload + user list in Postgres via
    // AuthRepository, then returns a freshly signed JWT plus the first
    // userId the frontend should authenticate.
    InitSessionResult initSession(const std::string& payloadJson, const std::vector<int>& userIds);

    // Decodes and verifies a JWT's signature/expiry, returning the session
    // state. Throws std::runtime_error (401-worthy) on invalid/expired/tampered tokens.
    AuthSession decode(const std::string& token) const;

    // Re-serializes and re-signs a (mutated) session back into a JWT string.
    std::string encode(const AuthSession& session) const;

    // true if userId is part of this session's user list.
    bool containsUser(const AuthSession& session, int userId) const;

    // Prerequisite check used by the vocal and fingerprint controllers before
    // they even attempt verification.
    bool isFactorDone(const AuthSession& session, int userId, AuthFactor factor) const;

    // Marks a factor as complete for a user. Mutates session in place; caller
    // still needs to call encode() to get the updated token string.
    void markFactorDone(AuthSession& session, int userId, AuthFactor factor);

    // Walks userIds in order and returns the first one that isn't fully
    // authenticated on all three factors, or -1 if every user is done.
    int nextPendingUserId(const AuthSession& session) const;

    // Call this once nextPendingUserId() returns -1 for a session: fetches the
    // persisted payload from Postgres via AuthRepository::getSessionPayload,
    // POSTs it to the configured /submit endpoint, and marks the session
    // submitted on success. Returns false (without throwing) on any HTTP or
    // network failure so the controller can decide how to respond/retry.
    bool finalizeSession(const std::string& sessionId) const;

private:
    std::shared_ptr<AuthRepository> authRepository_;
    std::string jwtSecret_;
    std::string submitEndpointUrl_;
};
