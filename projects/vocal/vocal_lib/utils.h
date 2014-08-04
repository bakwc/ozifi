#pragma once

#include <string>
#include <projects/vocal/vocal_lib/vocal.pb.h>
#include "defines.h"

namespace NVocal {

std::pair<std::string, std::string> GetLoginHost(const std::string& login);
bool GoodLogin(const std::string& login);

std::string RegisterResultToString(ERegisterResult res);

std::string LoginResultToString(ELoginResult res);

}
