// <colordlg.cpp> -*- coding: cp932 -*-
//
// Project mandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
//
// - TColorDialog - 色設定ダイアログ
//

#include <owl/pch.h>
#pragma hdrstop

// my
#include "colordlg.h"

// our
#include "fltrngvl.h"           // TFloatRangeValidator

// owl
#include <owl/checkbox.h>       // owl::TCheckBox
#include <owl/chooseco.h>       // owl::TChooseColorDialog
#include <owl/edit.h>           // owl::TEdit

// boost
#include <boost/scope_exit.hpp> // BOOST_SCOPE_EXIT_ALL

// std
#include <sstream>              // std::ostringstream

// resource IDs
#include "mandel.rh"

using namespace std;
using namespace owl;

DIAG_DEFINE_GROUP_INIT("MANDEL.INI", COLORDLG, true, 1);

//------------------------------------------------------------
//
// class TColorDialog - 色設定ダイアログ
//

static void InitValue(TComboBoxData& cbd)
{
    cbd.AddStringItem(_T("0"), TColorMap::cmvAlways0);
    cbd.AddStringItem(_T("255"), TColorMap::cmvAlways255);
    cbd.AddStringItem(_T("0→255"), TColorMap::cmv0to255);
    cbd.AddStringItem(_T("255→0"), TColorMap::cmv255to0);
    cbd.AddStringItem(_T("0→255→0"), TColorMap::cmv0to255to0);
    cbd.AddStringItem(_T("255→0→255"), TColorMap::cmv255to0to255);
    cbd.Select(2);
}

static tstring to_tstring_fixed(double x, int precision = 0)
{
    tostringstream oss;
    oss << std::fixed << std::setprecision(precision) << x;
    return oss.str();
}

TColorDialog::TXferBuffer::TXferBuffer()
{
    InitValue(RValue);
    InitValue(GValue);
    InitValue(BValue);
    _tcscpy_s(RRotation, to_tstring_fixed(1, TColorDialog::RotationPrecision).c_str());
    _tcscpy_s(GRotation, to_tstring_fixed(8, TColorDialog::RotationPrecision).c_str());
    _tcscpy_s(BRotation, to_tstring_fixed(64, TColorDialog::RotationPrecision).c_str());
    _tcscpy_s(RNoDivergence, to_tstring(0).c_str());
    _tcscpy_s(GNoDivergence, to_tstring(0).c_str());
    _tcscpy_s(BNoDivergence, to_tstring(0).c_str());
    SameAsLatest = 0;
}

static int GetSelectedData(const TComboBoxData& cbd)
{
    int i = const_cast<TComboBoxData&>(cbd).GetSelIndex();
    const auto& datas = const_cast<TComboBoxData&>(cbd).GetItemDatas();
    if (datas.empty())
        return -1;
    if (i < 0 || datas.size() <= i)
        i = 0;
    return datas[i];
}

TColorMap TColorDialog::TXferBuffer::GetColorMap() const
{
    return TColorMap(GetSelectedData(RValue), _tstof(RRotation),
                     GetSelectedData(GValue), _tstof(GRotation),
                     GetSelectedData(BValue), _tstof(BRotation),
                     TColor(_tstoi(RNoDivergence),
                            _tstoi(GNoDivergence),
                            _tstoi(BNoDivergence)),
                     SameAsLatest != 0);
}

static void SetSelectData(TComboBoxData& cbd, int data)
{
    const auto& datas = const_cast<TComboBoxData&>(cbd).GetItemDatas();
    for (int i = 0; i < datas.size(); i++)
        if (datas[i] == data) {
            cbd.Select(i);
            break;
        }
}

void TColorDialog::TXferBuffer::SetColorMap(const TColorMap& m)
{
    SetSelectData(RValue, m.GetRValue_()); 
    SetSelectData(GValue, m.GetGValue_()); 
    SetSelectData(BValue, m.GetBValue_());
    _tcscpy_s(RRotation, to_tstring_fixed(m.GetRRotation(), RotationPrecision).c_str());
    _tcscpy_s(GRotation, to_tstring_fixed(m.GetGRotation(), RotationPrecision).c_str());
    _tcscpy_s(BRotation, to_tstring_fixed(m.GetBRotation(), RotationPrecision).c_str());
    //
    _tcscpy_s(RNoDivergence, to_tstring(m.GetNoDivergence().Red()).c_str());
    _tcscpy_s(GNoDivergence, to_tstring(m.GetNoDivergence().Green()).c_str());
    _tcscpy_s(BNoDivergence, to_tstring(m.GetNoDivergence().Blue()).c_str());
    SameAsLatest = m.GetNoDivergenceSameAsLatest();
}

//static
TColorDialog::TXferBuffer TColorDialog::Data;

DEFINE_RESPONSE_TABLE1(TColorDialog, TDialog)
    EV_COMMAND(IDC_RANDOM, CmRandom),
    EV_COMMAND(IDC_NODIVERGENCESELECT, CmNoDivergenceSelect),
    EV_CBN_SELCHANGE(IDC_RVALUE, EvDivergenceChange),
    EV_CBN_SELCHANGE(IDC_GVALUE, EvDivergenceChange),
    EV_CBN_SELCHANGE(IDC_BVALUE, EvDivergenceChange),
    EV_EN_KILLFOCUS(IDC_RROTATION, EvDivergenceChange),
    EV_EN_KILLFOCUS(IDC_GROTATION, EvDivergenceChange),
    EV_EN_KILLFOCUS(IDC_BROTATION, EvDivergenceChange),
    EV_EN_KILLFOCUS(IDC_RNODIVERGENCE, EvNoDivergenceChange),
    EV_EN_KILLFOCUS(IDC_GNODIVERGENCE, EvNoDivergenceChange),
    EV_EN_KILLFOCUS(IDC_BNODIVERGENCE, EvNoDivergenceChange),
    EV_BN_CLICKED(IDC_SAMEASLATEST, EvSameAsLatestBNClicked),
END_RESPONSE_TABLE;

TColorDialog::TColorDialog(TWindow* parent)
    : TDialog(parent, IDD_COLOR), SampleBitmap(0), FirstTransferSetData(true), LockResponse(0)
{
    new TComboBox(this, IDC_RVALUE);
    auto rRotationEdit = new TEdit(this, IDC_RROTATION, size(Data.RRotation));
    new TComboBox(this, IDC_GVALUE);
    auto gRotationEdit = new TEdit(this, IDC_GROTATION, size(Data.GRotation));
    new TComboBox(this, IDC_BVALUE);
    auto bRotationEdit = new TEdit(this, IDC_BROTATION, size(Data.BRotation));
    auto rNoDivergenceEdit = new TEdit(this, IDC_RNODIVERGENCE, size(Data.RNoDivergence));
    auto gNoDivergenceEdit = new TEdit(this, IDC_GNODIVERGENCE, size(Data.GNoDivergence));
    auto bNoDivergenceEdit = new TEdit(this, IDC_BNODIVERGENCE, size(Data.BNoDivergence));
    new TCheckBox(this, IDC_SAMEASLATEST);
    SamplePosition = new TStatic(this, IDC_SAMPLEPOSITION);
    Sample = new TStatic(this, IDC_SAMPLE);
    SetTransferBuffer(&Data);
    TRACEX(COLORDLG, 1, "TComboBox::TComboBox|TransferBufferSize=" << GetTransferBufferSize());
    rRotationEdit->SetValidator(new TFloatRangeValidator(0.001, 999.999, RotationPrecision));
    gRotationEdit->SetValidator(new TFloatRangeValidator(0.001, 999.999, RotationPrecision));
    bRotationEdit->SetValidator(new TFloatRangeValidator(0.001, 999.999, RotationPrecision));
    rNoDivergenceEdit->SetValidator(new TRangeValidator(0, 255));
    gNoDivergenceEdit->SetValidator(new TRangeValidator(0, 255));
    bNoDivergenceEdit->SetValidator(new TRangeValidator(0, 255));
}

TColorDialog::~TColorDialog()
{
    delete SampleBitmap;
}

static void SetHeight(TWindow& w, int h = 150)
{
    TRect r = w.GetWindowRect();
    w.SetWindowPos(0, 0, 0, r.Width(), h, SWP_NOZORDER | SWP_NOMOVE);
}

bool TColorDialog::Create()
{
    bool result = TDialog::Create();
    //
    return result;
}

void TColorDialog::SetupWindow()
{
    TDialog::SetupWindow();
    //
    SetHeight(TWindow{GetDlgItem(IDC_RVALUE)});
    SetHeight(TWindow{GetDlgItem(IDC_GVALUE)});
    SetHeight(TWindow{GetDlgItem(IDC_BVALUE)});
}

uint TColorDialog::Transfer(void* buffer, TTransferDirection direction)
{
    uint r;
    {
        ++LockResponse;
        BOOST_SCOPE_EXIT_ALL(this){ --LockResponse; };
        r = TDialog::Transfer(buffer, direction);
    }
    //
    if (direction == tdSetData && FirstTransferSetData) {
        FirstTransferSetData = false;
        DataToSample();
        UpdateState();
    }
    //
    return r;
}

void TColorDialog::CmRandom()
{
    TColorMap cm;
    cm.SetRandom();
    Data.SetColorMap(cm);
    TransferData(tdSetData);
    UpdateSample();
    UpdateState();
}

void TColorDialog::CmNoDivergenceSelect()
{
    TransferData(tdGetData);
    TColorMap cm = Data.GetColorMap();
    TRACEX(COLORDLG, 1, _T("TColorDialog::CmNoDivergenceSelect|cm=") << cm.Encode());
    TChooseColorDialog::TData data{CC_RGBINIT | CC_FULLOPEN, cm.GetNoDivergence(), true};
    TChooseColorDialog dialog{this, data};
    if (dialog.Execute() == IDOK) {
        cm.SetNoDivergence(data.Color);
        TRACEX(COLORDLG, 1, _T("TColorDialog::CmNoDivergenceSelect|cm=") << cm.Encode());
        Data.SetColorMap(cm);
        TransferData(tdSetData);
        UpdateSample();
    }
}

void TColorDialog::EvDivergenceChange()
{
    if (LockResponse) return;
    UpdateNoDivergence();
    UpdateSample();
}

void TColorDialog::EvNoDivergenceChange()
{
    if (LockResponse) return;
    UpdateSample();
}

void TColorDialog::EvSameAsLatestBNClicked()
{
    if (LockResponse) return;
    TRACEX(COLORDLG, 1, "TColorDialog::EvSameAsLatestBNClicked|checked=" << IsDlgButtonChecked(IDC_SAMEASLATEST));
    UpdateNoDivergence();
    UpdateSample();
    UpdateState();
}

void TColorDialog::DataToSample()
{
    if (!SampleBitmap) {
        TRect crect = SamplePosition->GetClientRect();
        TRACEX(COLORDLG, 1, "TColorDialog::UpdateSample|crect=" << crect);
        SampleBitmap = new TBitmap(TScreenDC(), crect.Width(), crect.Height());
    }
    // SampleBitmap に描く
    {
        TSize s = SampleBitmap->Size();
        TRACEX(COLORDLG, 1, "TColorDialog::UpdateSample|SampleBitmapSize=" << s);
        TMemoryDC dc(*SampleBitmap);
        dc.FillRect(0, 0, s.cx, s.cy, TBrush(TColor::LtBlue));
        auto m = Data.GetColorMap();
        TRACEX(COLORDLG, 1, "TColorDialog::DataToSample|m.encode=" << m.Encode());
        TRACEX(COLORDLG, 1, "TColorDialog::DataToSample|m.encode.encode=" << TColorMap(m.Encode()).Encode());
        for (int x = 0; x < s.cx; x++) {
            double value = x / (s.cx * 0.85);
            if (value >= 1)
                value = 1;
            TColor c = m(value);
            TPen pen{c};
            dc.SelectObject(pen);
            dc.MoveTo(x, 0);
            dc.LineTo(x, s.cy);
            dc.RestorePen();
        }
    } 
   //
    Sample->SetImage(*SampleBitmap);
    Sample->Invalidate();
    UpdateWindow();
}

void TColorDialog::UpdateSample()
{
    TransferData(tdGetData);
    DataToSample();
}

void TColorDialog::UpdateState()
{
    bool enable = !IsDlgButtonChecked(IDC_SAMEASLATEST);
    ::EnableWindow(GetDlgItem(IDC_RNODIVERGENCE), enable);
    ::EnableWindow(GetDlgItem(IDC_GNODIVERGENCE), enable);
    ::EnableWindow(GetDlgItem(IDC_BNODIVERGENCE), enable);
    ::EnableWindow(GetDlgItem(IDC_NODIVERGENCESELECT), enable);
}

void TColorDialog::UpdateNoDivergence()
{
    if (IsDlgButtonChecked(IDC_SAMEASLATEST)) {
        TransferData(tdGetData);
        TColorMap cm = Data.GetColorMap();
        cm.SetNoDivergenceSameAsLatest();
        Data.SetColorMap(cm);
        TransferData(tdSetData);
    }
}

//
// end of class TColorDialog - 色設定ダイアログ
//
//------------------------------------------------------------

// end of <colordlg.cpp>
