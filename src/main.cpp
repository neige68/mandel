// <main.cpp> -*- coding: cp932 -*-
//
// Project GoldenMandel
// Copyright (C) 2022 neige68
// https://github.com/neige68/mandel
// This program is released under license GPLv3
//
// - main - ���C��
//

#include <owl/pch.h>
#pragma hdrstop

// our
#include "colordlg.h"           // TColorDialog
#include "drawwndw.h"           // TDrawWindow
#include "misc.h"
#include "version.h"

// owl
#include <owl/applicat.h>       // owl::TAplication
#include <owl/decframe.h>       // owl::TDecoratedFrame
#include <owl/docview.rh>       // CM_FILESAVEAS ���p
#include <owl/opensave.h>       // owl::TOpenSaveDialog
#include <owl/profile.h>        // owl::TProfile
#include <owl/scroller.h>       // owl::TScroller
#include <owl/string.h>         // owl::TString

// ms
#include <gdiplus.h>            // Gdiplus::Bitmap

// std
#include <sstream>              // std::ostringstream
#include <string>               // std::string

// resource IDs
#include "mandel.rh"

using namespace std;
using namespace owl;

DIAG_DEFINE_GROUP_INIT("GoldenMandel.INI", MAIN, true, 1);

//------------------------------------------------------------

// �G���R�[�_���擾
//
// ��: GetEncoderClsid(L"image/jpeg", &clsidJpegCodec);
//
// return: �������͔񕉂̒l, �G���[���� -1
static
int GetEncoderClsid(const std::wstring& format, CLSID* pClsid)
{
    UINT num = 0;          // number of image encoders
    UINT size = 0;         // size of the image encoder array in bytes
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure
    std::vector<char> buf(size);
    auto pImageCodecInfo = reinterpret_cast<Gdiplus::ImageCodecInfo*>(&buf[0]);
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j) {
        if (pImageCodecInfo[j].MimeType == format) {
            if (pClsid)
                *pClsid = pImageCodecInfo[j].Clsid;
            return j;           // Success
        }    
    }
    return -1;  // Failure
}

//------------------------------------------------------------
//
// class TAboutDialog - �o�[�W�������_�C�A���O
//

class TAboutDialog : public TDialog {

    // *** constructor ***
public:
    TAboutDialog(TWindow* parent) : TDialog(parent, IDD_ABOUT) {}

    // *** owl override ***
    void SetupWindow() override;
    
};

// �����������Ɉړ�
static void CenteringDlgItem(owl::TWindow& parent, int id)
{
    owl::TRect clientRect = parent.GetClientRect();
    HWND hwnd = parent.GetDlgItem(id);
    owl::TRect scRect;
    ::GetWindowRect(hwnd, &scRect);
    owl::TRect ccRect = parent.MapScreenToClient(scRect);
    owl::TScreenDC dc;
    dc.SelectObject(owl::TFont(parent.GetWindowFont()));
    owl::tstring text = parent.GetDlgItemText(id);
    int width = dc.GetTextExtent(text, text.length()).cx;
    ::SetWindowPos(hwnd, 0,
                   (clientRect.Width() - width) / 2, ccRect.top, width + 2, ccRect.Height(),
                   SWP_NOZORDER);
}

void TAboutDialog::SetupWindow() 
{
    owl::TDialog::SetupWindow();
    //
    tstring product{owl::TString(VER_PRODUCTNAME)};
    tstring version{owl::TString(VER_FILEVERSION_STR)};
    tstring title = product + _T(" Ver.") + version;
    if (sizeof(void*) == 8)
        title += _T(" [Win64]");
    SetDlgItemText(IDC_TITLE, title);
    SetDlgItemText(IDC_COPYRIGHT, owl::TString(VER_LEGALCOPYRIGHT));
    // �����������Ɉړ�
    CenteringDlgItem(*this, IDC_TITLE);
    CenteringDlgItem(*this, IDC_COPYRIGHT);
    CenteringDlgItem(*this, IDC_URL);
    CenteringDlgItem(*this, IDC_LICENSE);
};

//------------------------------------------------------------
//
// class TMyMainWindow - ���C���E�B���h�E
//

class TMyMainWindow : public TDecoratedFrame {

    // *** constructor ***
public:
    TMyMainWindow(tstring exePath, LPCTSTR title);

    // *** response ***
    void CmFileSaveAs();
    void CmColor();
    void CmColorRandom();
    void CmHelpAbout() { TAboutDialog(this).Execute(); }
    void CmHelpReadMe();

    // *** data ***
private:
    tstring ExePath;
    TDrawWindow* DrawWindow;
        
    DECLARE_RESPONSE_TABLE(TMyMainWindow);
};

DEFINE_RESPONSE_TABLE1(TMyMainWindow, TDecoratedFrame)
    EV_COMMAND(CM_FILESAVEAS, CmFileSaveAs),
    EV_COMMAND(CM_COLOR, CmColor),
    EV_COMMAND(CM_COLORRANDOM, CmColorRandom),
    EV_COMMAND(CM_HELPREADME, CmHelpReadMe),
    EV_COMMAND(CM_HELPABOUT, CmHelpAbout),
END_RESPONSE_TABLE;

TMyMainWindow::TMyMainWindow(tstring exePath, LPCTSTR title) :
    TDecoratedFrame(0, title, DrawWindow = new TDrawWindow), ExePath(exePath)
{
    AssignMenu(MAIN_MENU);
    Attr.AccelTable = MAIN_ACCELERATORS;
    uint modeIndicators = TStatusBar::SizeGrip;
    TStatusBar* statusBar = new TStatusBar(this, TGadget::Recessed, modeIndicators,
                                           new TDefaultGuiFont{TDefaultGuiFont::sfiStatus});
    Insert(*statusBar, TDecoratedFrame::Bottom);
    DrawWindow->SetStatusBar(statusBar);
}

static std::string GdiplusStatusToString(const Gdiplus::Status status)
{
    switch (status) {
    case Gdiplus::Ok: return "Ok";
    case Gdiplus::GenericError: return "GenericError";
    case Gdiplus::InvalidParameter: return "InvalidParameter";
    case Gdiplus::OutOfMemory: return "OutOfMemory";
    case Gdiplus::ObjectBusy: return "ObjectBusy";
    case Gdiplus::InsufficientBuffer: return "InsufficientBuffer";
    case Gdiplus::NotImplemented: return "NotImplemented";
    case Gdiplus::Win32Error: return "Win32Error";
    case Gdiplus::Aborted: return "Aborted";
    case Gdiplus::FileNotFound: return "FileNotFound";
    case Gdiplus::ValueOverflow: return "ValueOverflow";
    case Gdiplus::AccessDenied: return "AccessDenied";
    case Gdiplus::UnknownImageFormat: return "UnknownImageFormat";
    case Gdiplus::FontFamilyNotFound: return "FontFamilyNotFound";
    case Gdiplus::FontStyleNotFound: return "FontStyleNotFound";
    case Gdiplus::NotTrueTypeFont: return "NotTrueTypeFont";
    case Gdiplus::UnsupportedGdiplusVersion: return "UnsupportedGdiplusVersion";
    case Gdiplus::GdiplusNotInitialized: return "GdiplusNotInitialized";
    case Gdiplus::PropertyNotFound: return "PropertyNotFound";
    case Gdiplus::PropertyNotSupported: return "PropertyNotSupported";
    default: return "Status Type Not Found.";
    }
}

static void SetFileName(TOpenSaveDialog::TData& data, LPCTSTR fname)
{
    size_t len = _tcslen(fname) + 1;
    if (len > static_cast<size_t>(data.MaxPath)) {
        delete data.FileName;
        data.FileName = 0;
        data.FileName = new TCHAR[len];
        data.MaxPath = len;
    }
    _tcscpy(data.FileName, fname);
}

void TMyMainWindow::CmFileSaveAs()
{
    tstring filter = _T("JPEG file (*.jpg)|*.jpg");
    static TOpenSaveDialog::TData FileData(OFN_PATHMUSTEXIST
                                           | OFN_HIDEREADONLY
                                           | OFN_OVERWRITEPROMPT
                                           // | OFN_NOCHANGEDIR
                                           | OFN_ENABLESIZING,
                                           const_cast<TCHAR*>(filter.c_str()), 0, 0,
                                           const_cast<TCHAR*>(_T("jpg")));
    // �t�@�C��������
    time_t t = time(0);
    TCHAR bufFileName[34+1];
    _tcsftime(bufFileName, size(bufFileName), _T("GoldenMandel %Y-%m-%d %H%M%S"), localtime(&t));
    SetFileName(FileData, bufFileName);
    //
    if (TFileSaveDialog(this, FileData).Execute() != IDOK) {
        if (FileData.Error != 0)
            throw runtime_error(TString(ErrorMessage(FileData.Error)));
        return;
    }
    // GDI �̃r�b�g�}�b�v�ɃR�s�[
    TRect rect = DrawWindow->GetClientRect();
    TBitmap bmp(TScreenDC(), rect.Width(), rect.Height());
    {
        TClientDC srcDC{*DrawWindow};
        TMemoryDC dstDC{bmp};
        dstDC.BitBlt(0, 0, rect.Width(), rect.Height(), srcDC, 0, 0);
    }
    // GDI+ �̃r�b�g�}�b�v����
    unique_ptr<Gdiplus::Bitmap> gdip_bmp{Gdiplus::Bitmap::FromHBITMAP(bmp, 0)};
    // ���^�f�[�^�ǉ�
    const PROPID PropertyTagWindowsSpecificTitle = 0x9c9b;
    unique_ptr<Gdiplus::PropertyItem> propertyItemWinTitle(new Gdiplus::PropertyItem);
    {
        auto* propertyItem = propertyItemWinTitle.get();
        wchar_t propertyValue[] = L"GoldenMandel";
        propertyItem->id = PropertyTagWindowsSpecificTitle;
        propertyItem->length = sizeof(propertyValue); // L'\0' �I�[�܂ރo�C�g��
        propertyItem->type = PropertyTagTypeByte;
        propertyItem->value = propertyValue;
        if (Gdiplus::Status s = gdip_bmp->SetPropertyItem(propertyItem))
            throw runtime_error{GdiplusStatusToString(s) + " @Gdiplus::Image::SetPropertyItem."};
    }
    const PROPID PropertyTagWindowsSpecificComment = 0x9c9c;
    wstring propertyValueWinComment;
    unique_ptr<Gdiplus::PropertyItem> propertyItemWinComment(new Gdiplus::PropertyItem);
    {
        auto* propertyItem = propertyItemWinComment.get();
        propertyValueWinComment = TString{DrawWindow->GetDescription()};
        propertyItem->id = PropertyTagWindowsSpecificComment;
        propertyItem->length = propertyValueWinComment.size() * 2 + 2; // L'\0' �I�[�܂ރo�C�g��
        propertyItem->type = PropertyTagTypeByte;
        propertyItem->value = &propertyValueWinComment.at(0);
        if (Gdiplus::Status s = gdip_bmp->SetPropertyItem(propertyItem))
            throw runtime_error{GdiplusStatusToString(s) + " @Gdiplus::Image::SetPropertyItem."};
    }
    // �Ƃ肠���� Jpeg
    CLSID clsidJpegCodec;
    if (GetEncoderClsid(L"image/jpeg", &clsidJpegCodec) < 0)
        throw runtime_error{"JPEG �G���R�[�_�[������܂���."};
    if (Gdiplus::Status s = gdip_bmp->Save(FileData.FileName, &clsidJpegCodec))
        throw runtime_error{GdiplusStatusToString(s) + "(GDI+ Save)"};
}

void TMyMainWindow::CmColor()
{
    if (TColorDialog(this).Execute() == IDOK)
        DrawWindow->SetColorMap(TColorDialog::Data.GetColorMap());
}

void TMyMainWindow::CmColorRandom()
{
    TColorMap cm;
    cm.SetRandom();
    TColorDialog::Data.SetColorMap(cm);
    DrawWindow->SetColorMap(cm);
}

void TMyMainWindow::CmHelpReadMe()
{
    filesystem::path path{ExePath};
    filesystem::path toOpen;
    for (int i = 0; i < 3; i++) { // �k���ăt�@�C����T��
        path = path.parent_path();
        if (path.empty()) break;
        auto readme = path / L"readme.txt";
        if (filesystem::exists(readme)) {
            toOpen = readme;
            break;
        }
        readme = path / L"binary-readme.txt";
        if (filesystem::exists(readme)) {
            toOpen = readme;
            break;
        }
    }
    if (!toOpen.empty())
        ShellExecute(*this, _T("open"), toOpen.c_str(), nullptr, nullptr, SW_SHOW);
}

//------------------------------------------------------------
//
// class TMyApp - �A�v���P�[�V����
//

class TMyApp : public TApplication {
public:
    TMyApp(const tstring& exePath, const tstring& title) : TApplication(title), ExePath(exePath) {}
    void InitMainWindow() override {
        TRACEX(MAIN, 10, "TMyApp::InitMainWindow");
        auto frame = new TMyMainWindow(ExePath, GetName());
        frame->SetIcon(this, IDI_APP);
        frame->SetIconSm(this, IDI_APP);
        SetMainWindow(frame);
    }
private:
    tstring ExePath;
};

//------------------------------------------------------------

int OwlMain(int argc, _TCHAR* argv[])
{
    tstring title = _T("GoldenMandel");
    if (sizeof(void*) == 8)
        title += _T(" [64bit]");
#if !defined(NDEBUG)
    title += _T(" [debug]");
#endif        
    //
    int result = -1;
    try {
        StartDebugMonitor(title.c_str());
        locale::global(locale(locale::classic(), "", locale::ctype));
        TProfileFileName::Instance().Init(argv[0], _T("GoldenMandel.ini"));
        // �������s�X���b�h�����擾���Ă݂�
        TProfile profile(_T("Draw"), TProfileFileName::Instance().GetFilePath());
        int threads = profile.GetInt(_T("Threads"), GetNumberOfProcessors());
        TRACEX(MAIN, 0, "OwlMain|threads=" << threads);
        //
        TMyApp(argv[0], title).Run();
    }
    catch (const exception& x) {
        MessageBox(0, TString(x.what()), title.c_str(), MB_OK);
    }
    catch (...) {
        MessageBox(0, _T("Unexpected exception."), title.c_str(), MB_ICONSTOP | MB_OK);
    }
    return result;
}

//------------------------------------------------------------

// end of <main.cpp>
