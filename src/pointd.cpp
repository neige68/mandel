// <pointd.cpp> -*- coding: cp932 -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TPointD - TPoint �� double �l��
//

#include <owl/pch.h>
#pragma hdrstop

// my
#include "pointd.h"

using namespace std;
using namespace owl;

//------------------------------------------------------------
//
// struct TPointD - TPoint �� double �l��
//

tostream& operator << (tostream& os, const TPointD& o)
{
    return os << _T('(') << o.x << _T(',') << o.y << _T(')');
}

//------------------------------------------------------------

// end of <pointd.cpp>
