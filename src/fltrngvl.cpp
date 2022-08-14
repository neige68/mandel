// <fltrngvl.cpp> -*- coding: cp932 -*-
//
// Project mandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TFloatRangeValidator - 浮動小数点範囲バリデータ
//

#include <owl/pch.h>
#pragma hdrstop

// my
#include "fltrngvl.h"
#include "fltrngvl.rh"

using namespace std;
using namespace owl;

DIAG_DEFINE_GROUP_INIT("MANDEL.INI", FLTRNGVL, true, 1);

//------------------------------------------------------------
//
// class TFloatRangeValidator - 浮動小数点範囲バリデータ
//

void TFloatRangeValidator::Error(TWindow* window)
{
    PRECONDITION(window);
    window->FormatMessageBox(window->LoadString(IDS_VALNOTINFLOATRANGE),
                             window->LoadString(IDS_VALCAPTION),
                             MB_ICONERROR | MB_OK,
                             ValueToString(Range.Min).c_str(),
                             ValueToString(Range.Max).c_str());
}

static double ParseDouble(tistream& is)
{
    double value = 0;
    is >> value;
    if (!is.fail() && !is.eof()) {
        if (!is.eof()) {
            value = 0;
            is.setstate(is.failbit);
        }
    }
    return value;
}

bool TFloatRangeValidator::IsValid(LPCTSTR s)
{
    PRECONDITION(s);
    if (!TFilterValidator::IsValid(s))
        return false;
    tistringstream is{s};
    const double value = ParseDouble(is);
    return !is.fail() && Range.Min <= value && value <= Range.Max;
}

tstring TFloatRangeValidator::ValueToString(double x) const
{
    tostringstream oss;
    oss << std::fixed << std::setprecision(Precision) << x;
    return oss.str();
}

//
// end of class TFloatRangeValidator - 浮動小数点範囲バリデータ
//
//------------------------------------------------------------

// end of <fltrngvl.cpp>
