#pragma once
#include <string>

using namespace std;

string LoadFile(const string& fileName);
void SaveFile(const string& fileName, const string& data);
wstring UTF8ToWide(const string& text);
string WideToUTF8(const wstring& text);
string RecodeText(string text, const string& from, const string& to);
size_t CalcWordsCount(const string& text);
size_t CalcDigitCount(const string& text);
size_t CalcPunctCount(const string& text);
size_t CalcSentencesCount(const string& text);
bool HasPunct(const string& text);
bool isdatedelim(char c);
bool issentbreak(char c);
bool iswordsymbol(wchar_t symbol);
string NormalizeText(const string& text, bool hard = true);
string ImproveText(const string& text);
string DecodeHtmlEntities(const string &text);