// <colormap.h> -*- coding: cp932; mode: c++ -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TColorMap - [0,1] の double 値から色へのマップ
//

#ifndef colormap_h
#define colormap_h

// std
#include <string>               // std::string

//------------------------------------------------------------
//
// class TColorMap - [0,1] の double 値から色へのマップ
//

class TColorMap {

    // *** type ***
public:
    enum TValue {      
        cmvAlways0 = 0,
            // ここの間は常にその値
        cmvAlways255 = 255,
        cmv0to255,
        cmv255to0,
        cmv0to255to0,
        cmv255to0to255,
        cmvBound,
    };

    // *** constructors ***
    TColorMap(int rValue = cmv0to255, double rRotation = 1,
              int gValue = cmv0to255, double gRotation = 1,
              int bValue = cmvAlways0, double bRotation = 1,
              owl::TColor noDivergence = owl::TColor{255, 255, 0},
              bool noDivergenceSameAsLatest = false);
    TColorMap(const owl::tstring& encoded) { Decode(encoded); }

    // *** functions ***
    // 範囲外の値があれば有効値に設定する
    void Validate();
    // [0,1] に対する発散色
    owl::TColor DivergenceColor(double value) const;
    // [0,1) に対する発散色, 1 に対する非発散色
    owl::TColor operator()(double value) const;
    // 文字列エンコード
    owl::tstring Encode() const;
    operator owl::tstring() const { return Encode(); }
    void Decode(const owl::tstring&);
    // 乱数で設定
    void SetRandom();
    // 非発散色の設定
    void SetNoDivergence(const owl::TColor& c) { NoDivergence = c; }
    // 非発散色は最遅発散色と同じであるかを設定
    // true のときは、非発散色に最遅発散色を設定
    void SetNoDivergenceSameAsLatest(bool flag = true);

    // *** getters ***
    TValue GetRValue_() const { return RValue; }
    double GetRRotation() const { return RRotation; }
    TValue GetGValue_() const { return GValue; }
    double GetGRotation() const { return GRotation; }
    TValue GetBValue_() const { return BValue; }
    double GetBRotation() const { return BRotation; }
    owl::TColor GetNoDivergence() const { return NoDivergence; }
    bool GetNoDivergenceSameAsLatest() const { return NoDivergenceSameAsLatest; }

    // *** data ***
private:
    // 発散色
    TValue RValue;
    double RRotation;
    TValue GValue;
    double GRotation;
    TValue BValue;
    double BRotation;
    // 非発散色
    owl::TColor NoDivergence;
    bool NoDivergenceSameAsLatest; // 最遅発散色と同じ
    
};

//------------------------------------------------------------

#endif // colormap_h

// end of <colormap.h>
