// <misc.h> -*- coding: cp932; mode: c++ -*-
//
// Project mandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
//
// - misc
//

#ifndef misc_h
#define misc_h

// owl
#include <owl/defs.h>           // owl::tstring

// std c++
#include <string>               // std::string

//------------------------------------------------------------
//
// sigleton class TProfileFileName
//

class TProfileFileName {

    // *** static ***
    static owl::tstring FilePath;
public:
    static TProfileFileName& Instance();

    // *** constructor & destructor ***
private:
    TProfileFileName() {}
public:
    ~TProfileFileName() {}

    // *** functions ***
    void Init(owl::tstring exePath, owl::tstring iniFileName);
    owl::tstring GetFilePath() const { return FilePath; }
    
};

//------------------------------------------------------------
//
// functions
//

// mt19937 を使用した[0, n) の一様乱数生成
int random(int n);

// GetLastError の値を対応するメッセージに変換する
owl::tstring ErrorMessage(DWORD id, DWORD dwLanguageId = 0);

// モジュールのファイル名
owl::tstring GetModuleFileNameString(HMODULE hModule = 0);

// DbgMon に監視をスタートさせる
void StartDebugMonitor(LPCTSTR title);

// アプリケーションハング判定時間 [msec]
DWORD GetHungAppTimeout() noexcept;

// プロセッサ(論理コア)の数
size_t GetNumberOfProcessors();

//------------------------------------------------------------

#endif // misc_h

// end of <misc.h>
