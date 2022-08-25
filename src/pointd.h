// <pointd.h> -*- coding: cp932; mode: c++ -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TPointD - TPoint の double 値版
//

#ifndef pointd_h
#define pointd_h

#include <owl/private/strmdefs.h> // owl::tostream

//------------------------------------------------------------
//
// struct TPointD - TPoint の double 値版
//

struct TPointD {
    double x;
    double y;
    TPointD(double x_ = 0, double y_ = 0) : x(x_), y(y_) {}
    friend owl::tostream& operator << (owl::tostream& os, const TPointD& o);
};

//------------------------------------------------------------

#endif // pointd_h

// end of <pointd.h>
