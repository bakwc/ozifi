#pragma once

#include <contrib/htmlcxx/html/ParserDom.h>

bool GetCharset(tree<htmlcxx::HTML::Node>::iterator it, std::string& charset);
