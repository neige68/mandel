// <version.h> -*- coding: cp932; mode: c++ -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - version - �o�[�W������`
//

#ifndef version_h
#define version_h

//------------------------------------------------------------

#define VER_COMPANYNAME                 "neige68"
#define VER_PRODUCTNAME                 "GoldenMandel"

// �����[�X�E�p�b�`�ԍ�
#define VER_RELEASE_STR                 "0"
#define VER_RELEASE                     0
#define VER_PATCH_STR                   "" // "" �������� ".x"
#define VER_PATCH                       0

// ���i�o�[�W����
#define VER_PRODUCTVERSION_STR          "1.0"
#define VER_PRODUCTVERSION              1,0,VER_RELEASE,VER_PATCH

// �t�@�C���o�[�W����
#define VER_FILEVERSION_STR             "1.0." VER_RELEASE_STR VER_PATCH_STR
#define VER_FILEVERSION                 1,0,VER_RELEASE,VER_PATCH

// �t�@�C������
#define VER_FILEDESCRIPTION             "�}���f���u���W���`��"

// ���쌠�\��
#define VER_LEGALCOPYRIGHT_LASTYEAR     "2022"
#define VER_LEGALCOPYRIGHT              "Copyright (C) " VER_LEGALCOPYRIGHT_LASTYEAR " " VER_COMPANYNAME

//------------------------------------------------------------

#endif // version_h

// end of <version.h>
