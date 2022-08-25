// <colormap.cpp> -*- coding: cp932 -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TColorMap - [0,1] の double 値から色へのマップ
//

#include <owl/pch.h>
#pragma hdrstop

// my
#include "colormap.h"

// our
#include "misc.h"               // random

// owl
#include <owl/color.h>          // owl::TColor

using namespace std;
using namespace owl;

DIAG_DEFINE_GROUP_INIT("GoldenMandel.INI", COLORMAP, true, 1);

//------------------------------------------------------------
//
// class TColorMap - [0,1] の double 値から色へのマップ
//

TColorMap::TColorMap(int rValue, double rRotation,
                     int gValue, double gRotation,
                     int bValue, double bRotation,
                     owl::TColor noDivergence,
                     bool noDivergenceSameAsLatest) :
    RValue(static_cast<TValue>(rValue)), RRotation(rRotation),
    GValue(static_cast<TValue>(gValue)), GRotation(gRotation),
    BValue(static_cast<TValue>(bValue)), BRotation(bRotation),
    NoDivergence(noDivergence), NoDivergenceSameAsLatest(noDivergenceSameAsLatest)
{
    Validate();
}

static void ValidateValue(TColorMap::TValue& value)
{
    if (value < 0) value = TColorMap::cmvAlways0;
    if (value >= TColorMap::cmvBound) value = static_cast<TColorMap::TValue>(TColorMap::cmvBound - 1);
}

static void ValidateRotation(double& rotation)
{
    if (rotation <= 0.0) rotation = 0.0001;
    if (rotation > 99.9999) rotation = 99.9999;
}

void TColorMap::Validate()
{
    ValidateValue(RValue);
    ValidateValue(GValue);
    ValidateValue(BValue);
    ValidateRotation(RRotation);
    ValidateRotation(GRotation);
    ValidateRotation(BRotation);
}

static int part(TColorMap::TValue value, double rotation, double r)
{
    if (value <= TColorMap::cmvAlways255)
        return value;
    double ipart;
    double rr = modf(r * rotation, &ipart); // [0,1)
    if (rr == 0 && r != 0)
        rr = 1.0;
    switch (value) {
    case TColorMap::cmv0to255:
        return 255 * rr;
    case TColorMap::cmv255to0:
        return 255 * (1 - rr);
    case TColorMap::cmv0to255to0:
        if (rr < 0.5)
            return 255 * (rr * 2); // rr=0 => 0, rr=0.5 => 255
        return 255 * (1 - rr) * 2; // rr=0.5 => 255, rr=1 => 0
    case TColorMap::cmv255to0to255:
        if (rr < 0.5)
            return 255 * (0.5 - rr) * 2; // rr=0 => 255, rr=0.5 => 0
        return 255 * (rr - 0.5) * 2; // rr=0.5 => 0, rr=1 => 255
    }
    return 128;
}

TColor TColorMap::DivergenceColor(double value) const
{
    return TColor(part(RValue, RRotation, value),
                  part(GValue, GRotation, value),
                  part(BValue, BRotation, value));
}

TColor TColorMap::operator()(double value) const
{
    if (value == 1)
        return NoDivergence;
    return DivergenceColor(value);
}

tstring TColorMap::Encode() const
{
    tostringstream oss;
    oss << RValue << _T(',') << RRotation
        << _T(',') << GValue << _T(',') << GRotation 
        << _T(',') << BValue << _T(',') << BRotation
        << _T(',') << int(NoDivergence.Red())
        << _T(',') << int(NoDivergence.Green())
        << _T(',') << int(NoDivergence.Blue())
        << _T(',') << int(NoDivergenceSameAsLatest);
    return oss.str();
}

void TColorMap::Decode(const tstring& encoded)
{
    tistringstream iss(encoded);
    TCHAR c;
    int r = 0, g = 0, b = 0, s = 0;
    iss >> reinterpret_cast<int&>(RValue) >> c >> RRotation
        >> c >> reinterpret_cast<int&>(GValue) >> c >> GRotation
        >> c >> reinterpret_cast<int&>(BValue) >> c >> BRotation
        >> c >> r
        >> c >> g
        >> c >> b
        >> c >> s;
    NoDivergence = TColor{r, g, b};
    NoDivergenceSameAsLatest = (s != 0);
}

static void RandomValue(TColorMap::TValue& value)
{
    int n = TColorMap::cmvAlways255 + random(TColorMap::cmvBound - TColorMap::cmvAlways255 + 1);
    if (n == TColorMap::cmvBound)
        n = TColorMap::cmvAlways0;
    value = static_cast<TColorMap::TValue>(n);
}

static void RandomRotation(double& rotation)
{
    double candidate[] = {
        1,
        1.5,
        2,
        4,
        8,
        64,
    };
    rotation = candidate[random(size(candidate))];
}

void TColorMap::SetRandom()
{
    for (;;) {
        RandomValue(RValue); RandomRotation(RRotation);
        RandomValue(GValue); RandomRotation(GRotation);
        RandomValue(BValue); RandomRotation(BRotation);
        //
        NoDivergenceSameAsLatest = (random(2) != 0);
        if (NoDivergenceSameAsLatest)
            SetNoDivergenceSameAsLatest();
        else if (random(2) != 0)
            NoDivergence = TColor::White;
        else
            NoDivergence = TColor::Black;
        //
        if (RValue > cmvAlways255
            || GValue > cmvAlways255
            || BValue > cmvAlways255) // どれからが単色でなければ抜ける
            break;
    }
}

void TColorMap::SetNoDivergenceSameAsLatest(bool flag)
{
    NoDivergenceSameAsLatest = flag;
    if (NoDivergenceSameAsLatest) {
        NoDivergence = DivergenceColor(1);
        TRACEX(COLORMAP, 1, "TColorMap::SetNoDivergenceSameAsLatest|encode=" << Encode());
    }
}

//
// end of class TColorMap - [0,1] の double 値から色へのマップ
//
//------------------------------------------------------------

// end of <colormap.cpp>
