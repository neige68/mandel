// <drawwndw.h> -*- coding: cp932; mode: c++ -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TDrawWindow - 描画ウィンドウ
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
// class TDrawWindow - 描画ウィンドウ
//

// 4つの座標系
//
// * sc: スクリーン座標; 第1モニタの左上を原点、右下方向をプラスとする
//
// * cc: クライアントウィンドウ座標; TDrawWindow のクライアントウィンド
//   ウの左上を原点、右下方向をプラスとする
//
// * vc: 仮想ウィンドウ座標; クライアントウィンドウ座標にスクローラの
//   値 (XPos, YPos) を加算した座標
//
// * oc: 対象座標: 複素平面座標; 右上方向がプラス; 実数値なので
//   TPointD を使用
//

class TDrawWindow : public owl::TWindow {

    // *** types ***
public:
    typedef owl::uint uint;
    typedef owl::TPoint TPoint;

    // *** const ***
    // スクローラの上限
    const int vcsRange = 2 << 15;
    // スクローラの中央
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

    // 座標変換
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
    std::queue<owl::TRect> VCRectsToInvalidate;  // 後のタイマイベントで無効化する領域
    double Magnification;                        // 表示倍率
    TPoint VCBasePoint;                          // 仮想座標基準点
    TPointD OCBasePoint;                         // 対象座標基準点
    TPoint CCDragStart; // ドラッグ開始クライアント座標、未使用時(-1,-1)
    TPoint ScrollerPosDragStart; // ドラッグ開始時のスクローラ位置
    TColorMap ColorMap;
    int IterationLimit; // マンデルブロ集合生成漸化式繰り返し回数限度
    double DivergenceThresholdAbs; // マンデルブロ集合生成発散判定絶対値閾値
    DWORD TickDrew; // 描画にかかった時間[msec]

    DECLARE_RESPONSE_TABLE(TDrawWindow);
};

//------------------------------------------------------------

#endif // drawwndw_h

// end of <drawwndw.h>
