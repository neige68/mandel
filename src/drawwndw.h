// <drawwndw.h> -*- coding: cp932; mode: c++ -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TDrawWindow - �`��E�B���h�E
//

#ifndef drawwndw_h
#define drawwndw_h

// our
#include "pointd.h"             // TPointD
#include "colormap.h"           // TColorMap

// owl
#include <owl/scroller.h>       // owl::TScroller
#include <owl/statusba.h>       // owl::TStatusBar

// std
#include <queue>                // std::queue

//------------------------------------------------------------
//
// class TDrawWindow - �`��E�B���h�E
//

// 4�̍��W�n
//
// * sc: �X�N���[�����W; ��1���j�^�̍�������_�A�E���������v���X�Ƃ���
//
// * cc: �N���C�A���g�E�B���h�E���W; TDrawWindow �̃N���C�A���g�E�B���h
//   �E�̍�������_�A�E���������v���X�Ƃ���
//
// * vc: ���z�E�B���h�E���W; �N���C�A���g�E�B���h�E���W�ɃX�N���[����
//   �l (XPos, YPos) �����Z�������W
//
// * oc: �Ώۍ��W: ���f���ʍ��W; �E��������v���X; �����l�Ȃ̂�
//   TPointD ���g�p
//

class TDrawWindow : public owl::TWindow {

    // *** types ***
public:
    typedef owl::uint uint;
    typedef owl::TPoint TPoint;

    // *** const ***
    // �X�N���[���̏��
    const int vcsRange = 2 << 15;
    // �X�N���[���̒���
    const int vcsCenter = vcsRange / 2;

    // *** constructor ***
    TDrawWindow();

    // *** functions ***
    void SetStatusBar(owl::TStatusBar* statusBar) { StatusBar = statusBar; }
    TColorMap GetColorMap() const { return ColorMap; }
    void SetColorMap(const TColorMap& cm) { ColorMap = cm; Invalidate(); }
    owl::tstring GetDescription() const;
    void UpdateStatusBarText() const;
    int DrawMethod();

    // ���W�ϊ�
    using TWindow::MapScreenToClient;
    using TWindow::MapClientToScreen;
    TPoint MapClientToVirtual(const TPoint& ccPoint) const {
        return ccPoint.OffsetBy(Scroller->XPos, Scroller->YPos);
    }
    TPoint MapVirtualToClient(const TPoint& vcPoint) const {
        return vcPoint.OffsetBy(-Scroller->XPos, -Scroller->YPos);
    }
    TPointD MapVirtualToObject(const TPoint& vcPoint) const {
        return TPointD( (vcPoint.x - VCBasePoint.x) / Magnification + OCBasePoint.x,
                       -(vcPoint.y - VCBasePoint.y) / Magnification + OCBasePoint.y);
    }
    double MapVirtualToObjectX(int vcX) const {
        return (vcX - VCBasePoint.x) / Magnification + OCBasePoint.x;
    }
    double MapVirtualToObjectY(int vcY) const {
        return -(vcY - VCBasePoint.y) / Magnification + OCBasePoint.y;
    }
    TPoint MapObjectToVirtual(const TPointD& ocPoint) const {
        return TPoint( (ocPoint.x - OCBasePoint.x) * Magnification + VCBasePoint.x,
                      -(ocPoint.y - OCBasePoint.y) * Magnification + VCBasePoint.y);
    }

    // *** owl override ***
    bool IdleAction(long idleCount) override {
        if (!GetScroller())
            FirstSetScroller();
        return TWindow::IdleAction(idleCount);
    }
    void Paint(owl::TDC& dc, bool erase, owl::TRect& vcRect) override;
private:
    void PaintMethod10(owl::TDC& dc, owl::TRect& vcRect);
    void PaintMethod11(owl::TDC& dc, owl::TRect& vcRect);
    void PaintMethod20(owl::TDC& dc, owl::TRect& vcRect);
    void PaintMethod21(owl::TDC& dc, owl::TRect& vcRect);
    void PaintMethod22(owl::TDC& dc, owl::TRect& vcRect);
    void PaintMethod23(owl::TDC& dc, owl::TRect& vcRect);

    // *** response functions ***
public:
    void EvTimer(uint timerId);
    void EvMouseWheel(uint fwKeys, int zDelta, const TPoint& scPoint);
    void EvMouseMove(uint modKeys, const TPoint& ccPoint);
    void EvLButtonDblClk(uint modKeys, const TPoint& ccPoint);
    void EvRButtonDblClk(uint modKeys, const TPoint& ccPoint);
    void EvLButtonDown(uint modKeys, const TPoint& ccPoint);
    void EvKeyDown(uint key, uint repeatCount, uint flags);
    void CmViewRefresh() {
        Invalidate();
        UpdateWindow();
    }

    // *** private functions ***
private:
    void FirstSetScroller();
    void DoSetTimer();
    void DoKillTimer() {
        if (timer) {
            KillTimer(1);
            timer = false;
        }
    }
    TPoint GetClientCenter() const {
        owl::TRect ccRect = GetClientRect();
        return TPoint((ccRect.left + ccRect.right) / 2,
                      (ccRect.top + ccRect.bottom) / 2);
    }
    void Zoom(double rate, const TPoint& ccPoint);
    
    // ** data ***
private:
    owl::TStatusBar* StatusBar;
    bool timer;
    std::queue<owl::TRect> VCRectsToInvalidate;  // ��̃^�C�}�C�x���g�Ŗ���������̈�
    double Magnification;                        // �\���{��
    TPoint VCBasePoint;                          // ���z���W��_
    TPointD OCBasePoint;                         // �Ώۍ��W��_
    TPoint CCDragStart; // �h���b�O�J�n�N���C�A���g���W�A���g�p��(-1,-1)
    TPoint ScrollerPosDragStart; // �h���b�O�J�n���̃X�N���[���ʒu
    TColorMap ColorMap;
    int IterationLimit; // �}���f���u���W�������Q�����J��Ԃ��񐔌��x
    double DivergenceThresholdAbs; // �}���f���u���W���������U�����Βl臒l
    DWORD TickDrew; // �`��ɂ�����������[msec]

    DECLARE_RESPONSE_TABLE(TDrawWindow);
};

//------------------------------------------------------------

#endif // drawwndw_h

// end of <drawwndw.h>
