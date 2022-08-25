// <colormap.h> -*- coding: cp932; mode: c++ -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TColorMap - [0,1] �� double �l����F�ւ̃}�b�v
//

#ifndef colormap_h
#define colormap_h

// std
#include <string>               // std::string

//------------------------------------------------------------
//
// class TColorMap - [0,1] �� double �l����F�ւ̃}�b�v
//

class TColorMap {

    // *** type ***
public:
    enum TValue {      
        cmvAlways0 = 0,
            // �����̊Ԃ͏�ɂ��̒l
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
    // �͈͊O�̒l������ΗL���l�ɐݒ肷��
    void Validate();
    // [0,1] �ɑ΂��锭�U�F
    owl::TColor DivergenceColor(double value) const;
    // [0,1) �ɑ΂��锭�U�F, 1 �ɑ΂���񔭎U�F
    owl::TColor operator()(double value) const;
    // ������G���R�[�h
    owl::tstring Encode() const;
    operator owl::tstring() const { return Encode(); }
    void Decode(const owl::tstring&);
    // �����Őݒ�
    void SetRandom();
    // �񔭎U�F�̐ݒ�
    void SetNoDivergence(const owl::TColor& c) { NoDivergence = c; }
    // �񔭎U�F�͍Œx���U�F�Ɠ����ł��邩��ݒ�
    // true �̂Ƃ��́A�񔭎U�F�ɍŒx���U�F��ݒ�
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
    // ���U�F
    TValue RValue;
    double RRotation;
    TValue GValue;
    double GRotation;
    TValue BValue;
    double BRotation;
    // �񔭎U�F
    owl::TColor NoDivergence;
    bool NoDivergenceSameAsLatest; // �Œx���U�F�Ɠ���
    
};

//------------------------------------------------------------

#endif // colormap_h

// end of <colormap.h>
