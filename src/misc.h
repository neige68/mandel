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

// mt19937 ���g�p����[0, n) �̈�l��������
int random(int n);

// GetLastError �̒l��Ή����郁�b�Z�[�W�ɕϊ�����
owl::tstring ErrorMessage(DWORD id, DWORD dwLanguageId = 0);

// ���W���[���̃t�@�C����
owl::tstring GetModuleFileNameString(HMODULE hModule = 0);

// DbgMon �ɊĎ����X�^�[�g������
void StartDebugMonitor(LPCTSTR title);

// �A�v���P�[�V�����n���O���莞�� [msec]
DWORD GetHungAppTimeout() noexcept;

// �v���Z�b�T(�_���R�A)�̐�
size_t GetNumberOfProcessors();

//------------------------------------------------------------

#endif // misc_h

// end of <misc.h>
