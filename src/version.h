// <version.h> -*- coding: cp932; mode: c++ -*-
//
// Project mandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
//
// - version - バージョン定義
//

#ifndef version_h
#define version_h

//------------------------------------------------------------

#define VER_COMPANYNAME                 "neige68"
#define VER_PRODUCTNAME                 "mandel"

// リリース・パッチ番号
#define VER_RELEASE_STR                 "0"
#define VER_RELEASE                     0
#define VER_PATCH_STR                   "" // "" もしくは ".x"
#define VER_PATCH                       0

// 製品バージョン
#define VER_PRODUCTVERSION_STR          "0.0"
#define VER_PRODUCTVERSION              0,0,VER_RELEASE,VER_PATCH

// ファイルバージョン
#define VER_FILEVERSION_STR             "0.0." VER_RELEASE_STR VER_PATCH_STR
#define VER_FILEVERSION                 0,0,VER_RELEASE,VER_PATCH

// ファイル説明
#define VER_FILEDESCRIPTION             "マンデルブロ集合描画"

// 著作権表示
#define VER_LEGALCOPYRIGHT_LASTYEAR     "2022"
#define VER_LEGALCOPYRIGHT              "Copyright (C) " VER_LEGALCOPYRIGHT_LASTYEAR " " VER_COMPANYNAME

//------------------------------------------------------------

#endif // version_h

// end of <version.h>
