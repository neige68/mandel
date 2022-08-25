// <fltrngvl.h> -*- coding: cp932; mode: c++ -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TFloatRangeValidator - 浮動小数点範囲バリデータ
//

#ifndef fltrngvl_h
#define fltrngvl_h

// owl
#include <owl/validate.h>       // owl::TFilterValidator

//------------------------------------------------------------
//
// class TFloatRangeValidator - 浮動小数点範囲バリデータ
//

class TFloatRangeValidator : public owl::TFilterValidator {

    // *** type ***
public:
    struct TRange { double Min, Max; };

    // *** constructor ***
    TFloatRangeValidator(double minValue, double maxValue, int precision) :
        TFloatRangeValidator(TRange{minValue, maxValue}, precision) {}
    TFloatRangeValidator(const TRange& range, int precision) :
        owl::TFilterValidator(_T("0-9-.")), Precision(precision)
        { SetRange(range); }

    // *** functions ***
    void Error(owl::TWindow* window);
    bool IsValid(LPCTSTR str);
    owl::tstring ValueToString(double x) const;
    void SetRange(const TRange& range) { Range = range; }

    // *** data ***
protected:
    TRange Range;
    int Precision;
};

//------------------------------------------------------------

#endif // fltrngvl_h

// end of <fltrngvl.h>
