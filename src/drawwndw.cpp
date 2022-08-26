// <drawwndw.cpp> -*- coding: cp932 -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - TDrawWindow - 描画ウィンドウ
//

#include <owl/pch.h>
#pragma hdrstop

// my
#include "drawwndw.h"

// our
#include "misc.h"               // TProfileFileName

// owl
#include <owl/dibitmap.h>       // owl::TDiBitmap
#include <owl/profile.h>        // owl::TProfile

// std
#include <algorithm>            // std::for_each
#include <execution>            // std::execution::parallel_unsequenced_policy
#include <limits>               // std::numeric_limits
#include <thread>               // std::thread
#include <vector>               // std::vector

// resource IDs
#include "mandel.rh"

using namespace std;
using namespace owl;

DIAG_DEFINE_GROUP_INIT("GoldenMandel.INI", DRAWWNDW, true, 1);

//------------------------------------------------------------

// マンデルブロ集合生成漸化式 発散判定回数
// return : [0,iterationLimit) ; 発散判定された場合の反復回数
//          iterationLimit ; 発散判定されなかった場合
static int MandelbrotDivergenceIteration(double cx, double cy,
                                         int iterationLimit, // 反復回数
                                         double divergenceThresholdSqrAbs) // 発散判定絶対値二乗
{
    double x = 0, y = 0;
    for (int i = 0; i < iterationLimit; i++) {
        const double xx = x * x;
        const double yy = y * y;
        if (xx + yy > divergenceThresholdSqrAbs)
            return i;
        const double x1 = xx - yy + cx;
        const double y1 = 2 * x * y + cy;
        x = x1;
        y = y1;
    }
    return iterationLimit;
}

struct TMandelbrotDivergenceData {
    double x0_;
    double y0_;
    int result_;
    TMandelbrotDivergenceData() {}
    TMandelbrotDivergenceData(double x0, double y0) : x0_(x0), y0_(y0) {}
};

//------------------------------------------------------------
//
// class TDrawWindow - 描画ウィンドウ
//

DEFINE_RESPONSE_TABLE1(TDrawWindow, TWindow)
    EV_WM_TIMER,
    EV_COMMAND(CM_VIEWREFRESH, CmViewRefresh),
    EV_WM_MOUSEWHEEL,
    EV_WM_MOUSEMOVE,
    EV_WM_LBUTTONDBLCLK,
    EV_WM_RBUTTONDBLCLK,
    EV_WM_LBUTTONDOWN,
    EV_WM_KEYDOWN,
END_RESPONSE_TABLE;

TDrawWindow::TDrawWindow() :
    TWindow(static_cast<TWindow*>(nullptr)),
    StatusBar(0),
    timer(false),
    CCDragStart(-1, -1),
    TickDrew(0)
{
    Attr.Style |= WS_HSCROLL | WS_VSCROLL;
    Magnification = 200.0;
    IterationLimit = 512;
    DivergenceThresholdAbs = 2.0;
    SetCursor(0, IDC_HAND);
}

owl::tstring TDrawWindow::GetDescription() const
{
    tostringstream oss;
    TPoint ccPoint = GetClientCenter();
    TPoint vcPoint = MapClientToVirtual(ccPoint);
    TPointD ocPoint = MapVirtualToObject(vcPoint);
    oss << setprecision(numeric_limits<double>::digits10 + 1)
        << _T("Pos=(") << ocPoint.x << _T(',') << ocPoint.y << _T(')') << endl;
    oss << setprecision(numeric_limits<float>::digits10)
        << _T("Magnification=") << Magnification << endl;
    oss << _T("ColorMap=") << ColorMap.Encode();
    return oss.str();
}

void TDrawWindow::UpdateStatusBarText() const
{
    if (StatusBar) {
        // 座標変換
        TPoint scPoint = GetCursorPos();
        TPoint ccPoint = MapScreenToClient(scPoint);
        TPoint vcPoint = MapClientToVirtual(ccPoint);
        TPointD ocPoint = MapVirtualToObject(vcPoint); // 複素平面座標
        // マウスポインタ位置の色
        TColor color = TClientDC(*this).GetPixel(ccPoint);
        // 書式化
        tostringstream oss;
        oss << setprecision(numeric_limits<double>::digits10 + 1)
            << _T("Pos=(") << ocPoint.x << _T(',') << ocPoint.y << _T(')');
        oss << setprecision(numeric_limits<float>::digits10)
            << _T("; Ratio=") << Magnification;
        oss << _T("; Color=(") << color.Red() << _T(',') << color.Green() << _T(',') << color.Blue() << _T(')');
#ifndef NDEBUG
        oss << _T("; Scroller=(") << Scroller->XPos << _T(',') << Scroller->YPos << _T(')')
            << _T("; ccPos=") << ccPoint
            << _T("; vcPos=") << vcPoint;
#endif
        oss << _T("; DrewTime[msec]=") << TickDrew;
        StatusBar->SetWideHints(false);
        StatusBar->SetHintText(oss.str().c_str());
    }
}

int TDrawWindow::DrawMethod()
{
    TProfile profile(_T("Draw"), TProfileFileName::Instance().GetFilePath());
    int method = profile.GetInt(_T("Method"), 0);
    if (method == 0) {
        // TODO: OpenCL
        int threads = profile.GetInt(_T("Threads"), GetNumberOfProcessors());
        if (threads == 1)
            method = 1;
        else
            method = 2;
        profile.WriteInt(_T("Method"), method);
    }
    return method;
}

void TDrawWindow::Paint(owl::TDC& dc, bool erase, TRect& vcRect)
{
    static DWORD tickDrawStart = GetTickCount();
    static bool restToDraw = false;
    if (!restToDraw)
        tickDrawStart = GetTickCount();
    int method = DrawMethod();
    TRACEX(DRAWWNDW, 0, "TDrawWindow::Paint|method=" << method);
    switch (method) {
    case 10:
        PaintMethod10(dc, vcRect);
        break;
    case 1:
    case 11:
        PaintMethod11(dc, vcRect);
        break;
    case 20:
        PaintMethod20(dc, vcRect);
        break;
    case 21:
        PaintMethod21(dc, vcRect);
        break;
    case 22:
        PaintMethod22(dc, vcRect);
        break;
    case 2:
    case 23:
        PaintMethod23(dc, vcRect);
        break;
    default:
        TRACEX(DRAWWNDW, 1, "No Paint Method.");
    }
    restToDraw = !VCRectsToInvalidate.empty();
    if (!restToDraw)
        TickDrew = GetTickCount() - tickDrawStart;
}

void TDrawWindow::PaintMethod10(owl::TDC& dc, TRect& vcRect)
{
    TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod10");
    // プロファイル
    TProfile profile(_T("Draw"), TProfileFileName::Instance().GetFilePath());
    DWORD peekMessageInterval = profile.GetInt(_T("PeekMessageInterval"), 100);
    //
    DoKillTimer();
    // 引数 cvRect はスクローラ考慮の座標となっている
    TRect ccRect = GetClientRect();
    TRACEX(DRAWWNDW, 10, "TDrawWindow::PaintMethod10|vcRect=" << vcRect << "|ccRect=" << ccRect);
    const double divergenceThresholdSqrAbs = DivergenceThresholdAbs * DivergenceThresholdAbs; // 絶対値二乗を計算しておく
    DWORD tickTimeout = 0;
    for (int ivcY = vcRect.top; ivcY < vcRect.bottom; ++ivcY) {
        if (GetTickCount() >= tickTimeout) {
            MSG msg;
            if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
                TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod10-SetVCRectsToInvalidate|top=" << vcRect.top << "|ivcY=" << ivcY << "|botom=" << vcRect.bottom);
                VCRectsToInvalidate.push(TRect(vcRect.left, ivcY, vcRect.right, vcRect.bottom));
                break;
            }
            tickTimeout = GetTickCount() + peekMessageInterval;
        }
        // Bitmap オブジェクトに1行分描いてから転送
        TBitmap tmp_bmp{dc, vcRect.Width(), 1};
        TMemoryDC tmp_dc{tmp_bmp};
        for (int dx = 0; dx < vcRect.Width(); ++dx) {
            int ivcX = vcRect.left + dx;
            TPointD ocPoint = MapVirtualToObject(TPoint(ivcX, ivcY));
            int n = MandelbrotDivergenceIteration(ocPoint.x, ocPoint.y, IterationLimit, divergenceThresholdSqrAbs);
            tmp_dc.SetPixel(dx, 0, ColorMap(double(n) / IterationLimit));
        }
        dc.BitBlt(vcRect.left, ivcY, vcRect.Width(), 1, tmp_dc, 0, 0, SRCCOPY);
    }
    if (!VCRectsToInvalidate.empty())
        DoSetTimer();
}

void TDrawWindow::PaintMethod11(owl::TDC& dc, TRect& vcRect)
{
    TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod11");
    // プロファイル
    TProfile profile(_T("Draw"), TProfileFileName::Instance().GetFilePath());
    DWORD peekMessageInterval = profile.GetInt(_T("PeekMessageInterval"), 100);
    //
    DoKillTimer();
    // 引数 cvRect はスクローラ考慮の座標となっている
    TRect ccRect = GetClientRect();
    TRACEX(DRAWWNDW, 10, "TDrawWindow::PaintMethod11|vcRect=" << vcRect << "|ccRect=" << ccRect);
    const double divergenceThresholdSqrAbs = DivergenceThresholdAbs * DivergenceThresholdAbs; // 絶対値二乗を計算しておく
    DWORD tickTimeout = 0;
    for (int ivcY = vcRect.top; ivcY < vcRect.bottom; ++ivcY) {
        if (GetTickCount() >= tickTimeout) {
            MSG msg;
            if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
                TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod11-SetVCRectsToInvalidate|top=" << vcRect.top << "|ivcY=" << ivcY << "|botom=" << vcRect.bottom);
                VCRectsToInvalidate.push(TRect(vcRect.left, ivcY, vcRect.right, vcRect.bottom));
                break;
            }
            tickTimeout = GetTickCount() + peekMessageInterval;
        }
        // TDiBitmap(DIB セクション オブジェクト)に1行分描いてから転送
        TDiBitmap tmpBitmap{vcRect.Width(), 1, (numeric_limits<int>::max)()};
        auto bits = static_cast<BYTE*>(tmpBitmap.GetBits());
        for (int dx = 0; dx < vcRect.Width(); ++dx) {
            int ivcX = vcRect.left + dx;
            TPointD ocPoint = MapVirtualToObject(TPoint(ivcX, ivcY));
            int n = MandelbrotDivergenceIteration(ocPoint.x, ocPoint.y, IterationLimit, divergenceThresholdSqrAbs);
            TColor c = ColorMap(double(n) / IterationLimit);
            bits[4*dx+0] = c.Blue();
            bits[4*dx+1] = c.Green();
            bits[4*dx+2] = c.Red();
        }
        dc.BitBlt(vcRect.left, ivcY, vcRect.Width(), 1, TMemoryDC{tmpBitmap}, 0, 0, SRCCOPY);
    }
    if (!VCRectsToInvalidate.empty())
        DoSetTimer();
}

void TDrawWindow::PaintMethod20(owl::TDC& dc, TRect& vcRect)
{
    TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod20-enter");
    // プロファイル
    TProfile profile(_T("Draw"), TProfileFileName::Instance().GetFilePath());
    DWORD peekMessageInterval = profile.GetInt(_T("PeekMessageInterval"), 100);
    //
    DoKillTimer();
    // 引数 cvRect はスクローラ考慮の座標となっている
    TRect ccRect = GetClientRect();
    TRACEX(DRAWWNDW, 10, "TDrawWindow::PaintMethod20|vcRect=" << vcRect << "|ccRect=" << ccRect);
    queue<unique_ptr<TBitmap>> qBitmap;
    queue<unique_ptr<TMemoryDC>> qMemoryDC;
    queue<thread> qThread;
    const double divergenceThresholdSqrAbs = DivergenceThresholdAbs * DivergenceThresholdAbs; // 絶対値二乗を計算しておく
    DWORD tickTimeout = 0;
    for (int ivcY = vcRect.top; ivcY < vcRect.bottom; ++ivcY) {
        if (GetTickCount() >= tickTimeout) {
            MSG msg;
            if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
                TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod20"
                       << "|msg=" << msg.message << '=' << TMsgName(msg.message));
                TRACEX(DRAWWNDW, 0, "TDrawWindow::Paint-SetVCRectsToInvalidate"
                       << "|top=" << vcRect.top << "|ivcY=" << ivcY << "|botom=" << vcRect.bottom);
                VCRectsToInvalidate.push(TRect(vcRect.left, ivcY, vcRect.right, vcRect.bottom));
                break;
            }
            tickTimeout = GetTickCount() + peekMessageInterval;
        }
        qBitmap.push(make_unique<TBitmap>(dc, vcRect.Width(), 1));
        TBitmap* bitmap = qBitmap.back().get();
        qMemoryDC.push(make_unique<TMemoryDC>(*bitmap));
        TMemoryDC* mdc = qMemoryDC.back().get();
        qThread.push(thread([=]{
            vector<TMandelbrotDivergenceData> paradata;
            paradata.reserve(vcRect.right - vcRect.left);
            for (int ivcX = vcRect.left; ivcX < vcRect.right; ++ivcX) {
                TPointD ocPoint = MapVirtualToObject(TPoint(ivcX, ivcY));
                paradata.push_back(TMandelbrotDivergenceData(ocPoint.x, ocPoint.y));
            }
            for_each(execution::par_unseq,
                     paradata.begin(), paradata.end(),
                     [this, divergenceThresholdSqrAbs] (auto& i) {
                         i.result_ = MandelbrotDivergenceIteration(i.x0_, i.y0_, IterationLimit,
                                                                   divergenceThresholdSqrAbs);
                     });
            for (int dx = 0; dx < vcRect.Width(); ++dx) {
                int ivcX = vcRect.left + dx;
                int n = paradata[dx].result_;
                mdc->SetPixel(dx, 0, ColorMap(double(n) / IterationLimit));
            }
        }));
    }
    // ここより上の方が時間がかかっている
    // おそらくリソースの準備のせい
    TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod20-push-done");
    for (int ivcY = vcRect.top; ivcY < vcRect.bottom; ++ivcY) {
        if (qThread.empty()) break;
        qThread.front().join();
        dc.BitBlt(vcRect.left, ivcY, vcRect.Width(), 1, *qMemoryDC.front(), 0, 0, SRCCOPY);
        qThread.pop();
        qMemoryDC.pop();
        qBitmap.pop();
    }
    if (!VCRectsToInvalidate.empty())
        DoSetTimer();
    TRACEX(DRAWWNDW, 1, "TDrawWindow::PaintMethod20-leave");
}

// 同時スレッド数分だけ一時ビットマップなどを生成
void TDrawWindow::PaintMethod21(owl::TDC& dc, TRect& vcRect)
{
    TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod21-enter");
    // プロファイル
    TProfile profile(_T("Draw"), TProfileFileName::Instance().GetFilePath());
    int nthreads = profile.GetInt(_T("Threads"), GetNumberOfProcessors()) + 1;
    DWORD peekMessageInterval = profile.GetInt(_T("PeekMessageInterval"), 100);
    //
    DoKillTimer();
    // 引数 cvRect はスクローラ考慮の座標となっている
    TRect ccRect = GetClientRect();
    TRACEX(DRAWWNDW, 10, "TDrawWindow::PaintMethod21|vcRect=" << vcRect << "|ccRect=" << ccRect);
    // 同時実行スレッド数を取得してみる
    vector<unique_ptr<TBitmap>> vBitmap(nthreads);
    vector<shared_ptr<TMemoryDC>> vMemoryDC(nthreads);
    vector<thread> vThread(nthreads);
    const double divergenceThresholdSqrAbs = DivergenceThresholdAbs * DivergenceThresholdAbs; // 絶対値二乗を計算しておく
    DWORD tickTimeout = 0;
    int vcBottomToDraw = vcRect.bottom;
    for (int i = 0; i < vcBottomToDraw - vcRect.top + nthreads; i++) {
        // push
        int ivcY = vcRect.top + i;
        int iPushThread = i % nthreads;
        if (ivcY < vcBottomToDraw && GetTickCount() >= tickTimeout) {
            MSG msg;
            if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
                TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod21"
                       << "|msg=" << msg.message << '=' << TMsgName(msg.message));
                TRACEX(DRAWWNDW, 0, "TDrawWindow::Paint-SetVCRectsToInvalidate"
                       << "|top=" << vcRect.top << "|ivcY=" << ivcY << "|botom=" << vcRect.bottom);
                VCRectsToInvalidate.push(TRect(vcRect.left, ivcY, vcRect.right, vcRect.bottom));
                vcBottomToDraw = ivcY;
            }
            tickTimeout = GetTickCount() + peekMessageInterval;
        }
        if (ivcY < vcBottomToDraw) {
            if (!vBitmap[iPushThread]) {
                vBitmap[iPushThread].reset(new TBitmap{dc, vcRect.Width(), 1});
                vMemoryDC[iPushThread].reset(new TMemoryDC{*vBitmap[iPushThread].get()});
            }
            vThread[iPushThread] = thread([&](int ivcY, int iPushThread){
                vector<TMandelbrotDivergenceData> paradata;
                paradata.reserve(vcRect.right - vcRect.left);
                for (int ivcX = vcRect.left; ivcX < vcRect.right; ++ivcX) {
                    TPointD ocPoint = MapVirtualToObject(TPoint(ivcX, ivcY));
                    paradata.push_back(TMandelbrotDivergenceData(ocPoint.x, ocPoint.y));
                }
                for_each(execution::par_unseq,
                         paradata.begin(), paradata.end(),
                         [this, divergenceThresholdSqrAbs] (auto& i) {
                             i.result_ = MandelbrotDivergenceIteration(i.x0_, i.y0_, IterationLimit,
                                                                       divergenceThresholdSqrAbs);
                         });
                for (int dx = 0; dx < vcRect.Width(); ++dx) {
                    int ivcX = vcRect.left + dx;
                    int n = paradata[dx].result_;
                    vMemoryDC[iPushThread]->SetPixel(dx, 0, ColorMap(double(n) / IterationLimit));
                }
            }, ivcY, iPushThread);
        }
        // pop
        int ivcPopY = vcRect.top + i - nthreads + 1;
        int iPopThread = (i + 1) % nthreads;
        if (vcRect.top <= ivcPopY && ivcPopY < vcBottomToDraw) {
            vThread[iPopThread].join();
            dc.BitBlt(vcRect.left, ivcPopY, vcRect.Width(), 1, *vMemoryDC[iPopThread], 0, 0, SRCCOPY);
        }
    }
    if (!VCRectsToInvalidate.empty())
        DoSetTimer();
    TRACEX(DRAWWNDW, 1, "TDrawWindow::PaintMethod21-leave");
}

// 同時スレッド数分だけ一時ビットマップなどを生成
void TDrawWindow::PaintMethod22(owl::TDC& dc, TRect& vcRect)
{
    TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod22-enter");
    // プロファイル
    TProfile profile(_T("Draw"), TProfileFileName::Instance().GetFilePath());
    int nthreads = profile.GetInt(_T("Threads"), GetNumberOfProcessors()) + 1;
    int rowsPerThreads = profile.GetInt(_T("RowsPerThreads"), 1);
    if (rowsPerThreads <= 0)
        rowsPerThreads = (vcRect.Height() + nthreads - 1) / nthreads;
    DWORD peekMessageInterval = profile.GetInt(_T("PeekMessageInterval"), 100);
    //
    DoKillTimer();
    // 引数 cvRect はスクローラ考慮の座標となっている
    TRect ccRect = GetClientRect();
    TRACEX(DRAWWNDW, 10, "TDrawWindow::PaintMethod22|vcRect=" << vcRect << "|ccRect=" << ccRect);
    vector<unique_ptr<TBitmap>> vBitmap(nthreads);
    vector<shared_ptr<TMemoryDC>> vMemoryDC(nthreads);
    vector<thread> vThread(nthreads);
    vector<vector<TPointD>> ocPoints(nthreads, vector<TPointD>(vcRect.right - vcRect.left));
    vector<vector<int>> divergenceCounts(nthreads, vector<int>(vcRect.right - vcRect.left));
    const double divergenceThresholdSqrAbs = DivergenceThresholdAbs * DivergenceThresholdAbs; // 絶対値二乗を計算しておく
    DWORD tickTimeout = 0;
    int vcBottomToDraw = vcRect.bottom;
    int divThreads = (vcBottomToDraw - vcRect.top + rowsPerThreads - 1) / rowsPerThreads;
    for (int i = 0; i < divThreads + nthreads; i++) {
        // push
        int ivcY = vcRect.top + i * rowsPerThreads;
        int iPushThread = i % nthreads;
        if (ivcY < vcBottomToDraw && GetTickCount() >= tickTimeout) {
            MSG msg;
            if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
                TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod22"
                       << "|msg=" << msg.message << '=' << TMsgName(msg.message));
                TRACEX(DRAWWNDW, 0, "TDrawWindow::Paint-SetVCRectsToInvalidate"
                       << "|top=" << vcRect.top << "|ivcY=" << ivcY << "|botom=" << vcRect.bottom);
                VCRectsToInvalidate.push(TRect(vcRect.left, ivcY, vcRect.right, vcRect.bottom));
                vcBottomToDraw = ivcY;
            }
            tickTimeout = GetTickCount() + peekMessageInterval;
        }
        if (ivcY < vcBottomToDraw) {
            if (!vBitmap[iPushThread]) {
                vBitmap[iPushThread].reset(new TBitmap{dc, vcRect.Width(), rowsPerThreads});
                vMemoryDC[iPushThread].reset(new TMemoryDC{*vBitmap[iPushThread].get()});
            }
            vThread[iPushThread] = thread([&](int ivcY, int iPushThread){
                auto& ocPointsThread = ocPoints[iPushThread];
                auto& divergenceCountsThread = divergenceCounts[iPushThread];
                for (int dy = 0; dy < rowsPerThreads; ++dy) {
                    if (!(ivcY + dy < vcBottomToDraw)) break;
                    double ocPointY = MapVirtualToObjectY(ivcY + dy);
                    for (int ivcX = vcRect.left; ivcX < vcRect.right; ++ivcX) {
                        double ocPointX = MapVirtualToObjectX(ivcX);
                        ocPointsThread[ivcX - vcRect.left] = TPointD(ocPointX, ocPointY);
                    }
                    transform(ocPointsThread.begin(), ocPointsThread.end(), divergenceCountsThread.begin(),
                             [this, divergenceThresholdSqrAbs] (auto& i) {
                                 return MandelbrotDivergenceIteration(i.x, i.y, IterationLimit,
                                                                      divergenceThresholdSqrAbs);
                             });
                    for (int dx = 0; dx < vcRect.Width(); ++dx) {
                        int ivcX = vcRect.left + dx;
                        int n = divergenceCountsThread[dx];
                        vMemoryDC[iPushThread]->SetPixel(dx, dy, ColorMap(double(n) / IterationLimit));
                    }
                }
            }, ivcY, iPushThread);
        }
        // pop
        int ivcPopY = vcRect.top + (i - nthreads + 1) * rowsPerThreads;
        int iPopThread = (i + 1) % nthreads;
        if (vcRect.top <= ivcPopY && ivcPopY < vcBottomToDraw) {
            TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod22-join|ivcPopY=" << ivcPopY);
            vThread[iPopThread].join();
            int h = rowsPerThreads;
            if (ivcPopY + h >= vcBottomToDraw)
                h = vcBottomToDraw - ivcPopY;
            TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod22-bitblt|h=" << h);
            dc.BitBlt(vcRect.left, ivcPopY, vcRect.Width(), h, *vMemoryDC[iPopThread], 0, 0, SRCCOPY);
        }
    }
    if (!VCRectsToInvalidate.empty())
        DoSetTimer();
    TRACEX(DRAWWNDW, 1, "TDrawWindow::PaintMethod22-leave");
}

// 同時スレッド数分だけ一時ビットマップなどを生成
void TDrawWindow::PaintMethod23(owl::TDC& dc, TRect& vcRect)
{
    TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod23-enter");
    // プロファイル
    TProfile profile(_T("Draw"), TProfileFileName::Instance().GetFilePath());
    int nthreads = profile.GetInt(_T("Threads"), GetNumberOfProcessors()) + 1;
    int rowsPerThreads = profile.GetInt(_T("RowsPerThreads"), 1);
    if (rowsPerThreads <= 0)
        rowsPerThreads = (vcRect.Height() + nthreads - 1) / nthreads;
    DWORD peekMessageInterval = profile.GetInt(_T("PeekMessageInterval"), 100);
    //
    DoKillTimer();
    // 引数 cvRect はスクローラ考慮の座標となっている
    TRect ccRect = GetClientRect();
    TRACEX(DRAWWNDW, 10, "TDrawWindow::PaintMethod23|vcRect=" << vcRect << "|ccRect=" << ccRect);
    vector<unique_ptr<TDiBitmap>> vBitmap(nthreads);
    vector<thread> vThread(nthreads);
    vector<vector<TPointD>> ocPoints(nthreads, vector<TPointD>(vcRect.right - vcRect.left));
    vector<vector<int>> divergenceCounts(nthreads, vector<int>(vcRect.right - vcRect.left));
    const double divergenceThresholdSqrAbs = DivergenceThresholdAbs * DivergenceThresholdAbs; // 絶対値二乗を計算しておく
    DWORD tickTimeout = 0;
    int vcBottomToDraw = vcRect.bottom;
    int divThreads = (vcBottomToDraw - vcRect.top + rowsPerThreads - 1) / rowsPerThreads;
    for (int i = 0; i < divThreads + nthreads; i++) {
        // push
        int ivcY = vcRect.top + i * rowsPerThreads;
        int iPushThread = i % nthreads;
        if (ivcY < vcBottomToDraw && GetTickCount() >= tickTimeout) {
            MSG msg;
            if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
                TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod23"
                       << "|msg=" << msg.message << '=' << TMsgName(msg.message));
                TRACEX(DRAWWNDW, 0, "TDrawWindow::Paint-SetVCRectsToInvalidate"
                       << "|top=" << vcRect.top << "|ivcY=" << ivcY << "|botom=" << vcRect.bottom);
                VCRectsToInvalidate.push(TRect(vcRect.left, ivcY, vcRect.right, vcRect.bottom));
                vcBottomToDraw = ivcY;
            }
            tickTimeout = GetTickCount() + peekMessageInterval;
        }
        if (ivcY < vcBottomToDraw) {
            if (!vBitmap[iPushThread])
                vBitmap[iPushThread].reset(new TDiBitmap{vcRect.Width(), rowsPerThreads, (numeric_limits<int>::max)()});
            vThread[iPushThread] = thread([&](int ivcY, int iPushThread){
                auto& ocPointsThread = ocPoints[iPushThread];
                auto& divergenceCountsThread = divergenceCounts[iPushThread];
                for (int dy = 0; dy < rowsPerThreads; ++dy) {
                    if (!(ivcY + dy < vcBottomToDraw)) break;
                    double ocPointY = MapVirtualToObjectY(ivcY + dy);
                    for (int ivcX = vcRect.left; ivcX < vcRect.right; ++ivcX) {
                        double ocPointX = MapVirtualToObjectX(ivcX);
                        ocPointsThread[ivcX - vcRect.left] = TPointD(ocPointX, ocPointY);
                    }
                    transform(ocPointsThread.begin(), ocPointsThread.end(), divergenceCountsThread.begin(),
                             [this, divergenceThresholdSqrAbs] (auto& i) {
                                 return MandelbrotDivergenceIteration(i.x, i.y, IterationLimit,
                                                                      divergenceThresholdSqrAbs);
                             });
                    BYTE* bits = static_cast<BYTE*>(vBitmap[iPushThread]->GetBits());
                    // bottom-up DIB なので注意
                    BYTE* pColorByte = bits + 4 * (rowsPerThreads - dy - 1) * vcRect.Width();
                    for (int dx = 0; dx < vcRect.Width(); ++dx) {
                        int ivcX = vcRect.left + dx;
                        int n = divergenceCountsThread[dx];
                        TColor c = ColorMap(double(n) / IterationLimit);
                        *pColorByte++ = c.Blue();
                        *pColorByte++ = c.Green();
                        *pColorByte++ = c.Red();
                        *pColorByte++;
                    }
                }
            }, ivcY, iPushThread);
        }
        // pop
        int ivcPopY = vcRect.top + (i - nthreads + 1) * rowsPerThreads;
        int iPopThread = (i + 1) % nthreads;
        if (vcRect.top <= ivcPopY && ivcPopY < vcBottomToDraw) {
            TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod23-join|ivcPopY=" << ivcPopY);
            vThread[iPopThread].join();
            int h = rowsPerThreads;
            if (ivcPopY + h >= vcBottomToDraw)
                h = vcBottomToDraw - ivcPopY;
            TRACEX(DRAWWNDW, 0, "TDrawWindow::PaintMethod23-bitblt|h=" << h);
            dc.BitBlt(vcRect.left, ivcPopY, vcRect.Width(), h, TMemoryDC{*vBitmap[iPopThread]}, 0, 0, SRCCOPY);
        }
    }
    if (!VCRectsToInvalidate.empty())
        DoSetTimer();
    TRACEX(DRAWWNDW, 1, "TDrawWindow::PaintMethod23-leave");
}

void TDrawWindow::EvTimer(uint timerId)
{
    TRACEX(DRAWWNDW, 0, "TDrawWindow::EvTimer|VCRectsToInvalidate.size=" << VCRectsToInvalidate.size());
    if (VCRectsToInvalidate.empty())
        DoKillTimer();
    if (!VCRectsToInvalidate.empty()) {
        while (!VCRectsToInvalidate.empty()) {
            //TRACEX(DRAWWNDW, 0, "TDrawWindow::EvTimer|VCRectsToInvalidate.size=" << VCRectsToInvalidate.size());
            TRect vcRect = VCRectsToInvalidate.front();
            VCRectsToInvalidate.pop();
            // InvalidateRect(vcRect, false);
            TRACEX(DRAWWNDW, 10, "TDrawWindow::EvTimer-InvalidateRect|vcRect=" << vcRect);
            InvalidateRect(TRect(MapVirtualToClient(vcRect.TopLeft()), vcRect.Size()), false);
        }
        TRACEX(DRAWWNDW, 0, "TDrawWindow::EvTimer-UpdateWindow");
        UpdateWindow();
        TRACEX(DRAWWNDW, 0, "TDrawWindow::EvTimer-UpdateWindow-end");
    }
}

void TDrawWindow::EvMouseWheel(uint fwKeys, int zDelta, const TPoint& scPoint)
{
    // scPoint: 画面の左上隅を基準としたポインターの座標
    static int wheelContinuous = TProfile{_T("Draw"), TProfileFileName::Instance().GetFilePath()}.GetInt(_T("WheelContinuous"), 1);
    double rate = 1;
    if (wheelContinuous) // 連続値
        rate = pow(2, zDelta / WHEEL_DELTA);
    else {
        if (zDelta <= -WHEEL_DELTA) // down
            rate = 0.5;             // zoom out
        else if (zDelta >= WHEEL_DELTA) // up
            rate = 2;                   // zoom in
    }
    Zoom(rate, MapScreenToClient(scPoint));
}

void TDrawWindow::EvMouseMove(uint /*modKeys*/, const TPoint& ccPoint)
{
    if (CCDragStart != TPoint{-1, -1}) { // ドラッグ中
        GetScroller()->ScrollTo(ScrollerPosDragStart.x - ccPoint.x + CCDragStart.x,
                                ScrollerPosDragStart.y - ccPoint.y + CCDragStart.y);
    }
    //
    if (StatusBar)
        UpdateStatusBarText();
}

void TDrawWindow::EvLButtonDblClk(uint modKeys, const TPoint& ccPoint)
{
    double rate = 2.0;
    if (modKeys &= MK_SHIFT)
        rate = 0.5;
    Zoom(rate, ccPoint);
}

void TDrawWindow::EvRButtonDblClk(uint modKeys, const TPoint& ccPoint)
{
    double rate = 0.5;
    if (modKeys &= MK_SHIFT)
        rate = 2.0;
    Zoom(rate, ccPoint);
}

void TDrawWindow::EvLButtonDown(uint modKeys, const TPoint& ccPoint)
{
    TRACEX(DRAWWNDW, 1, _T("TDrawWindow::EvLButtonDown|ccPoint=") << ccPoint);
    CCDragStart = ccPoint;
    ScrollerPosDragStart = TPoint{GetScroller()->XPos, GetScroller()->YPos};
    TWindow::EvLButtonDown(modKeys, ccPoint);
    CCDragStart = TPoint{-1, -1};
    TRACEX(DRAWWNDW, 1, _T("TDrawWindow::EvLButtonDown-leave|ccPoint=") << ccPoint);
}

void TDrawWindow::EvKeyDown(uint key, uint repeatCount, uint flags)
{
    switch (key) {
    case VK_UP:
        GetScroller()->ScrollBy(0, -GetScroller()->YLine);
        break;
    case VK_DOWN:
        GetScroller()->ScrollBy(0, GetScroller()->YLine);
        break;
    case VK_LEFT:
        GetScroller()->ScrollBy(-GetScroller()->XLine, 0);
        break;
    case VK_RIGHT:
        GetScroller()->ScrollBy(GetScroller()->XLine, 0);
        break;
    case VK_PRIOR: // page up
        Zoom(2, GetClientCenter());
        break;
    case VK_NEXT: // page down
        Zoom(0.5, GetClientCenter());
        break;
    }
    TWindow::EvKeyDown(key, repeatCount, flags);
}

void TDrawWindow::FirstSetScroller()
{
    TRect crect = GetClientRect();
    TRACEX(DRAWWNDW, 1, _T("TDrawWindow::FirstSetScroller|crect=") << crect);
    // スクローラの範囲
    SetScroller(new TScroller(this, 1, 1, vcsRange, vcsRange));
    // 複素平面の原点と仮想座標の中央とウィンドウクライアント矩形の中央を合わせる
    TPoint ccCenter = TPoint(crect.left + crect.Width() / 2,
                             crect.top + crect.Height() / 2);
    TPoint ccPoint = ccCenter;
    VCBasePoint = TPoint(vcsCenter, vcsCenter);
    OCBasePoint = TPointD(0, 0);
    int xPos = VCBasePoint.x - ccPoint.x;
    int yPos = VCBasePoint.y - ccPoint.y;
    TRACEX(DRAWWNDW, 1, _T("TDrawWindow::FirstSetScroller|xPos=") << xPos << _T("|yPos=") << yPos);
    GetScroller()->ScrollTo(xPos, yPos);
    GetScroller()->SetPageSize();
    GetScroller()->XLine = 10;
    GetScroller()->YLine = 10;
    TRACEX(DRAWWNDW, 1, _T("TDrawWindow::FirstSetScroller")
           << _T("|XUnit=") << GetScroller()->XUnit
           << _T("|XLine=") << GetScroller()->XLine
           << _T("|XPage=") << GetScroller()->XPage
           << _T("|YUnit=") << GetScroller()->YUnit
           << _T("|YLine=") << GetScroller()->YLine
           << _T("|YPage=") << GetScroller()->YPage);
}

void TDrawWindow::DoSetTimer()
{
    static int timerInterval = TProfile{_T("Draw"), TProfileFileName::Instance().GetFilePath()}.GetInt(_T("TimerInterval"), 50);
    //
    if (!timer) {
        SetTimer(1, 50);
        timer = true;
    }
}

void TDrawWindow::Zoom(double rate, const TPoint& ccPoint)
{
    if (rate == 1.0) return;
    // ポインター位置から複素平面座標に
    TPoint vcPoint = MapClientToVirtual(ccPoint);
    TPointD ocPoint = MapVirtualToObject(vcPoint);
    TRACEX(DRAWWNDW, 0, _T("TDrawWindow::Zoom|rate=") << rate << _T("|ccPoint=") << ccPoint << _T("|vcPoint=") << vcPoint);
    // 倍率変更
    Magnification *= rate;
    TRACEX(DRAWWNDW, 0, _T("TDrawWindow::Zoom|ocPoint=") << ocPoint << _T("|Magnification=") << Magnification);
    // スクローラの範囲
    GetScroller()->SetRange(vcsRange, vcsRange);
    TRACEX(DRAWWNDW, 0, _T("TDrawWindow::Zoom|after-SetRange"));
    // ポインタ位置に相当する仮想座標と複素平面座標の組を記憶
    VCBasePoint = MapObjectToVirtual(ocPoint);
    OCBasePoint = ocPoint;
    int xPos = VCBasePoint.x - ccPoint.x;
    int yPos = VCBasePoint.y - ccPoint.y;
    Invalidate();
    TRACEX(DRAWWNDW, 0, _T("TDrawWindow::Zoom|after-Invalidate"));
    GetScroller()->ScrollTo(xPos, yPos);
    TRACEX(DRAWWNDW, 0, _T("TDrawWindow::Zoom-leave"));
}

//
// end of class TDrawWindow - 描画ウィンドウ
//
//------------------------------------------------------------

// end of <drawwndw.cpp>
