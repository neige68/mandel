// <misc.cpp> -*- coding: cp932 -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - misc
//

#include <owl/pch.h>
#pragma hdrstop

// my
#include "misc.h"

// owl
#include <owl/profile.h>        // owl::TProfile
#include <owl/registry.h>       // owl::TRegKey
#include <owl/string.h>         // owl::TString

// ms
#include "ShlObj_core.h"        // SHGetKnownFolderPath

// std
#include <filesystem>           // std::filesystem::path
#include <random>               // std::random_device
#include <sstream>              // std::ostringstream
#include <vector>               // std::vector

using namespace std;
using namespace owl;

DIAG_DEFINE_GROUP_INIT("GoldenMandel.INI", MISC, true, 1);

//------------------------------------------------------------
//
// sigleton class TProfileFileName
//

//static
owl::tstring TProfileFileName::FilePath;

//static
TProfileFileName& TProfileFileName::Instance()
{
    static TProfileFileName theInstance;
    return theInstance;
}

static bool IniWriteTest(owl::tstring iniPath)
{
    owl::TProfile profile(_T("Test"), iniPath);
    int r = random(32767), rr = -1;
    try {
        profile.WriteInt(_T("Test"), r);
        rr = profile.GetInt(_T("Test"), -1);
    }
    catch (...) {}
    return r == rr;
}

static wstring SHGetKnownFolderPathWString(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken)
{
    PWSTR pszPath = nullptr;
    wstring resultStr;
    HRESULT result = SHGetKnownFolderPath(rfid, dwFlags, hToken, &pszPath);
    if (result == S_OK)
        resultStr = pszPath;
    CoTaskMemFree(pszPath);
    switch (result) {
    case S_OK:
        break;
    case E_FAIL:
        throw runtime_error("Fail (SHGetKnownFolderPath).");
    case E_INVALIDARG:
        throw runtime_error("Invalid arg (SHGetKnownFolderPath).");
    default:
        throw runtime_error("Unexpected exception (SHGetKnownFolderPath).");
    }
    return resultStr;
}

void TProfileFileName::Init(owl::tstring exePath, owl::tstring iniFileName)
{
    TRACEX(MISC, 0, _T("TProfileFileName::Init-enter|exePath=") << exePath << _T("|iniFileName=") << iniFileName);
    auto iniPath = filesystem::path(exePath).parent_path() / iniFileName;
    TRACEX(MISC, 0, _T("TProfileFileName::Init|iniPath=") << iniPath);
    if (IniWriteTest(iniPath)) {
        FilePath = iniPath;
        TRACEX(MISC, 0, _T("TProfileFileName::Init-leave|FilePath=") << FilePath);
        return;
    }
    // FOLDERID_LocalAppData
    iniPath = filesystem::path(SHGetKnownFolderPathWString(FOLDERID_LocalAppData, 0, 0)) / iniFileName;
    if (IniWriteTest(iniPath)) {
        FilePath = iniPath;
        TRACEX(MISC, 0, _T("TProfileFileName::Init-leave|FilePath=") << FilePath);
        return;
    }
    throw runtime_error("INI ファイルのパスが決められません(TProfileFileName::Init).");
}

//
// end of sigleton class TProfileFileName
//
//------------------------------------------------------------
//
// functions
//

// mt19937 を使用した[0, n) の一様乱数生成
int random(int n)
{
    static random_device rd;
    static mt19937 gen(rd());
    return uniform_int_distribution<int>(0, n - 1)(gen);
}

// GetLastError の値を対応するメッセージに変換する
tstring ErrorMessage(DWORD id, DWORD dwLanguageId)
{
    LPTSTR buf = 0;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
                  | FORMAT_MESSAGE_MAX_WIDTH_MASK, 
                  0, id, dwLanguageId, (LPTSTR)&buf, 1, 0);
    tstring result(buf ? buf : _T(""));
    LocalFree(buf);
    return result;
}

// モジュールのファイル名
tstring GetModuleFileNameString(HMODULE hModule)
{
    vector<TCHAR> buf(MAX_PATH);
    for (;; buf.resize(buf.size() * 2)) {
        DWORD r = ::GetModuleFileName(hModule, &buf.at(0), buf.size());
        if (r == 0) { // error
            DWORD e = GetLastError();
            tostringstream msg;
            msg << _T("GetModuleFileName for ") << hModule << _T(" failed: ") << ErrorMessage(e) << endl;
            throw runtime_error(TString(msg.str()));
        }
        if (r < buf.size()) // バッファ十分
            break;
    }
    return tstring(&buf.at(0));
}

// DbgMon に監視をスタートさせる
void StartDebugMonitor(LPCTSTR title)
{
    static bool started = false;
    if (started) return;
    SendNotifyMessage(HWND_BROADCAST, RegisterWindowMessage(_T("StartDebugMonitor")), GetCurrentProcessId(), 0);
    //
    tstring moduleFileName;
    if (!title) {
        moduleFileName = GetModuleFileNameString();
        title = moduleFileName.c_str();
    }
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    TCHAR buf[48 + _MAX_PATH + 10 + 8 + 10 + 8];
    _stprintf_s(buf, _T("StartDebugMonitor %s(pid:%u=%#x,tid:%u=%#x)\r\n"), title, pid, pid, tid, tid);
    OutputDebugString(buf);
    //
    started = true;
}

// アプリケーションハング判定時間 [msec]
DWORD GetHungAppTimeout() noexcept
{
    DWORD result = 5000;
    try {
        owl::TRegKey reg(HKEY_CURRENT_USER, _T("Control Panel\\Desktop"), KEY_QUERY_VALUE, owl::TRegKey::NoCreate);
        LPCTSTR valueName = _T("HungAppTimeout");
        owl::TRegValue val(reg, valueName);
        if (val.GetDataType() == REG_SZ)
            result = _tstoi(LPCTSTR(val));
    }
    catch (...) {}
    return result;
}

// プロセッサ(論理コア)の数
size_t GetNumberOfProcessors() 
{ 
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    return systemInfo.dwNumberOfProcessors;
}

//------------------------------------------------------------

// end of <misc.cpp>
