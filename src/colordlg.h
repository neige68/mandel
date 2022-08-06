// <colordlg.h> -*- coding: cp932; mode: c++ -*-
//
// Project mandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
//
// - TColorDialog - 色設定ダイアログ
//

#ifndef colordlg_h
#define colordlg_h

// our
#include "colormap.h"           // TColorMap

// owl
#include <owl/combobox.h>       // owl::TComboxData
#include <owl/dialog.h>         // owl::TDialog
#include <owl/static.h>         // owl::TStatic

//------------------------------------------------------------
//
// class TColorDialog - 色設定ダイアログ
//

class TColorDialog : public owl::TDialog {

    // *** static ***
public:
#pragma pack(push, 1)
    static struct TXferBuffer {
        TXferBuffer();
        TColorMap GetColorMap() const;
        void SetColorMap(const TColorMap& m);
        //
        owl::TComboBoxData RValue;
        TCHAR RRotation[7+1];    // 999.999
        owl::TComboBoxData GValue;
        TCHAR GRotation[7+1];
        owl::TComboBoxData BValue;
        TCHAR BRotation[7+1];
        TCHAR RNoDivergence[3+1]; // 999
        TCHAR GNoDivergence[3+1];
        TCHAR BNoDivergence[3+1];
        owl::uint16 SameAsLatest;
    } Data;
#pragma pack(pop)

    // *** const ***
    static const int RotationPrecision = 3;

    // *** constructor & destructor ***
    TColorDialog(owl::TWindow* parent);
    ~TColorDialog();

    // *** OWL override ***
    bool Create() override;
    void SetupWindow() override;
    owl::uint Transfer(void* buffer, owl::TTransferDirection direction) override;

    // *** response ***
    void CmRandom();
    void CmNoDivergenceSelect();
    void EvDivergenceChange();
    void EvNoDivergenceChange();
    void EvSameAsLatestBNClicked();

    // *** private function ***
private:
    void DataToSample();
    void UpdateSample();
    void UpdateState();
    void UpdateNoDivergence();
    
    // *** data ***
    owl::TStatic* SamplePosition;
    owl::TStatic* Sample;
    owl::TBitmap* SampleBitmap;
    bool FirstTransferSetData;
    int LockResponse;

    DECLARE_RESPONSE_TABLE(TColorDialog);
};

//------------------------------------------------------------

#endif // colordlg_h

// end of <colordlg.h>
