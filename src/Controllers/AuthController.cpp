#include <Controllers/AuthController.h>
#include <utility.h>

AuthController::AuthController(std::shared_ptr<AuthService> authService)
:authService_(std::move(authService)) 
{
}

void AuthController::initAuthSession(const crow::request& req, crow::response& res) 
{
    auto json = crow::json::load(req.body);

    if (!json)
    {
        res.code = 400;
        set_cors(res);
        res.write("{\"error\": \"Invalid JSON\"}");
        return;
    }

    if (!json.has("jury"))
    {
        res.code = 400;
        set_cors(res);
        res.write("{\"error\": \"jury ID expected in the JSON !\"}");
        return;
    }

    // get juries
    std::vector<int> juriesIds;
    auto juries = json["jury"]; // the lists of the jury present at the soutenance
    int c = 0;
    for(const auto i:juries)
    {
        std::cout << c;
        try 
        {
            juriesIds.push_back(i.i());
            std::cout << "getting jurry " << i.i() << std::endl;

        } catch(std::exception error)
        {
            res.code = 400;
            set_cors(res);
            res.write("{\"error\": \"content of jury must be their IDS !\"}");
            return;
        }
        c+=1;
    }

    
    auto result = authService_->initSession(req.body, juriesIds);
    crow::json::wvalue result_json;
    result_json["uid"] = result.firstUserId;
    result_json["token"] = result.token;
    result_json["sessionId"] = result.sessionId;
    res.code = 200;
    res.set_header("Content-Type", "application/json");
    res.write(result_json.dump());
    res.end();
}
