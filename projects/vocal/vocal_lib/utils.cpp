#include <vector>
#include <boost/algorithm/string.hpp>
#include <utils/exception.h>

#include "utils.h"

using namespace std;

namespace NVocal {

std::pair<std::string, std::string> GetLoginHost(const std::string& login) {
    vector<string> result;
    boost::algorithm::split(result, login, boost::algorithm::is_any_of("@"));
    if (result.size() != 2) {
        throw UException("wrong login");
    }
    return pair<string, string>(result[0], result[1]);
}

string RegisterResultToString(ERegisterResult res) {
    switch (res) {
    case RR_ConnectionFailure: return "Connection Failure";
    case RR_Success: return "Success";
    case RR_WrongCaptcha: return "Wrong captcha";
    case RR_WrongLogin: return "Wrong login";
    }
    assert(false && "wrong result");
    return std::string("wrong result");
}

string LoginResultToString(ELoginResult res) {
    switch (res) {
    case LR_ConnectionFail: return "Connection Failure";
    case LR_Success: return "Success";
    case LR_WrongCaptcha: return "Wrong captcha";
    case LR_WrongUserOrPassword: return "Wrong login or password";
    }
    assert(false && "wrong result");
    return std::string("wrong result");
}

} // NVocal
