
#include <Windows.h>
#include <CommCtrl.h>


#include "main_window.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"
#include "win_image.h"
#include "media_setting_dialogue.h"
#include "tnfr.h"
#include "Resource.h"

#pragma comment(lib, "Comctl32.lib")

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_DEEPONE));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_ICON_DEEPONE);
    wcex.lpszClassName = m_class_name.c_str();
    wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_DEEPONE));

    if (::RegisterClassExW(&wcex))
    {
        m_hInstance = hInstance;

        m_hWnd = ::CreateWindowW(m_class_name.c_str(), m_window_name.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, nullptr, nullptr, hInstance, this);
        if (m_hWnd != nullptr)
        {
            return true;
        }
        else
        {
            std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
        }
    }
    else
    {
        std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

    return false;
}

int CMainWindow::MessageLoop()
{
    MSG msg;

    for (;;)
    {
        BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
        if (bRet > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        else if (bRet == 0)
        {
            /*ループ終了*/
            return static_cast<int>(msg.wParam);
        }
        else
        {
            /*ループ異常*/
            std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
            return -1;
        }
    }
    return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pThis != nullptr)
    {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate(hWnd);
    case WM_DESTROY:
        return OnDestroy();
    case WM_CLOSE:
        return OnClose();
    case WM_PAINT:
        return OnPaint();
    case WM_ERASEBKGND:
        return 1;
    case WM_KEYUP:
        return OnKeyUp(wParam, lParam);
    case WM_COMMAND:
        return OnCommand(wParam, lParam);
    case WM_TIMER:
        return OnTimer(wParam);
    case WM_MOUSEWHEEL:
        return OnMouseWheel(wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(wParam, lParam);
    case WM_MBUTTONUP:
        return OnMButtonUp(wParam, lParam);
    case EventMessage::kAudioPlayer:
        OnAudioPlayerEvent(static_cast<unsigned long>(lParam));
        break;
    case EventMessage::kVideoPlayer:
        OnVideoPlayerEvent(static_cast<unsigned long>(lParam));
        break;
    default:

        break;
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    InitialiseMenuBar();

    SetupDrawingInterval();

    m_pD2ImageDrawer = new CD2ImageDrawer(m_hWnd);

    m_pAudioPlayer = new CMfMediaPlayer();
    m_pAudioPlayer->SetPlaybackWindow(m_hWnd, EventMessage::kAudioPlayer);

    m_pVideoTransferor = new CMfVideoTransferor();
    m_pVideoTransferor->SetPlaybackWindow(m_hWnd, EventMessage::kVideoPlayer);
    m_pVideoTransferor->SwitchLoop();

    m_pD2TextWriter = new CD2TextWriter(m_pD2ImageDrawer->GetD2Factory(), m_pD2ImageDrawer->GetD2DeviceContext());
    m_pD2TextWriter->SetupOutLinedDrawing(L"C:\\Windows\\Fonts\\yumindb.ttf");

    m_pViewManager = new CViewManager(m_hWnd);

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    ::PostQuitMessage(0);

    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    EndThreadpoolTimer();

    ::KillTimer(m_hWnd, Timer::kText);

    if (m_pD2TextWriter != nullptr)
    {
        delete m_pD2TextWriter;
        m_pD2TextWriter = nullptr;
    }

    if (m_pD2ImageDrawer != nullptr)
    {
        delete m_pD2ImageDrawer;
        m_pD2ImageDrawer = nullptr;
    }

    if (m_pAudioPlayer != nullptr)
    {
        delete m_pAudioPlayer;
        m_pAudioPlayer = nullptr;
    }

    if (m_pVideoTransferor != nullptr)
    {
        delete m_pVideoTransferor;
        m_pVideoTransferor = nullptr;
    }

    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_class_name.c_str(), m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    if (m_pD2ImageDrawer != nullptr && m_pViewManager != nullptr)
    {
        bool bRet = false;

        if (m_bHasVideo && m_pVideoTransferor != nullptr)
        {
            SImageFrame s{};
            long long llCurrentTime = 0;
            bRet = m_pVideoTransferor->TransferVideoFrame(&s, &llCurrentTime);
            if (bRet)
            {
                bRet = m_pD2ImageDrawer->Draw(s, { m_pViewManager->GetXOffset(), m_pViewManager->GetYOffset() }, m_pViewManager->GetScale());
                if (bRet)
                {
                    StoreVideoFrame(llCurrentTime, s);
                }
            }
            else
            {
                long long llCurrentTime = m_pVideoTransferor->GetCurrentTimeInMilliSeconds();
                SImageFrame* s = ReStoreVideoFrame(llCurrentTime);
                if (s != nullptr)
                {
                    bRet = m_pD2ImageDrawer->Draw(*s, { m_pViewManager->GetXOffset(), m_pViewManager->GetYOffset() }, m_pViewManager->GetScale());
                }
            }
        }
        else
        {
            /*静画*/
            if (m_nImageIndex < m_imageFrames.size())
            {
                SImageFrame &s = m_imageFrames.at(m_nImageIndex);
                bRet = m_pD2ImageDrawer->Draw(s, { m_pViewManager->GetXOffset(), m_pViewManager->GetYOffset() }, m_pViewManager->GetScale());
            }
        }

        if (bRet)
        {
            if (!m_bTextHidden && m_pD2TextWriter != nullptr)
            {
                const std::wstring wstr = FormatCurrentText();
                m_pD2TextWriter->OutLinedDraw(wstr.c_str(), static_cast<unsigned long>(wstr.size()));
            }
            m_pD2ImageDrawer->Display();
        }
    }

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{

    return 0;
}
/*WM_KEYUP*/
LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case VK_ESCAPE:
        ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        break;
    case VK_UP:
        KeyUpOnForeFolder();
        break;
    case VK_DOWN:
        KeyUpOnNextFolder();
        break;
    case 'C':
        if (m_pD2TextWriter != nullptr)
        {
            m_pD2TextWriter->SwitchTextColour();
            UpdateScreen();
        }
        break;
    case 'T':
        m_bTextHidden ^= true;
        UpdateScreen();
        break;
    }
    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
    int wmId = LOWORD(wParam);
    int wmKind = LOWORD(lParam);
    if (wmKind == 0)
    {
        /*Menus*/
        switch (wmId)
        {
        case Menu::kOpenFolder:
            MenuOnOpenFolder();
            break;
        case Menu::kAudioLoop:
            MenuOnAudioLoop();
            break;
        case Menu::kAudioSetting:
            MenuOnAudioSetting();
            break;
        case Menu::kVideoPause:
            MenuOnVideoPause();
            break;
        case Menu::kVideoSetting:
            MenuOnVideoSetting();
            break;
        default:

            break;
        }
    }
    else
    {
        /*Controls*/
    }

    return 0;
}
/*WM_TIMER*/
LRESULT CMainWindow::OnTimer(WPARAM wParam)
{
    switch (wParam)
    {
    case Timer::kText:
        if (m_pAudioPlayer != nullptr)
        {
            if (m_pAudioPlayer->IsEnded())
            {
                AutoTexting();
            }
        }
        break;
    default:
        break;
    }
    return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
    WORD wKey = LOWORD(wParam);

    if (wKey == 0)
    {
        if (m_pViewManager != nullptr)
        {
            m_pViewManager->Rescale(iScroll > 0);
        }
    }

    if (wKey == MK_LBUTTON)
    {

    }

    if (wKey == MK_RBUTTON)
    {
        ShiftText(iScroll > 0);
    }

    return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    ::GetCursorPos(&m_CursorPos);

    /*When menu item is selected, WM_LBUTTONDOWN does not happen, but does WM_LBUTTONUP.*/
    m_bLeftDowned = true;

    return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);

    if (usKey == MK_RBUTTON && m_bBarHidden)
    {
        ::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_DOWN;
        ::SendInput(1, &input, sizeof(input));
    }

    if (usKey == 0 && m_bLeftDowned)
    {
        POINT pt{};
        ::GetCursorPos(&pt);
        int iX = m_CursorPos.x - pt.x;
        int iY = m_CursorPos.y - pt.y;

        if (iX == 0 && iY == 0)
        {
            ShiftImage(true);
        }
        {
            if (m_pViewManager != nullptr)
            {
                m_pViewManager->SetOffset(iX, iY);
            }
        }
    }

    m_bLeftDowned = false;

    return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);
    if (usKey == 0)
    {
        if (m_pViewManager != nullptr)
        {
            m_pViewManager->ResetZoom();
        }
    }

    if (usKey == MK_RBUTTON)
    {
        SwitchWindowMode();
    }

    return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
    HMENU hMenuFolder = nullptr;
    HMENU hMenuAudio = nullptr;
    HMENU hMenuVideo = nullptr;
    HMENU hMenuBar = nullptr;
    BOOL iRet = FALSE;

    if (m_hMenuBar != nullptr)return;

    /*フォルダ*/
    hMenuFolder = ::CreateMenu();
    if (hMenuFolder == nullptr)goto failed;
    iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kOpenFolder, "Open");
    if (iRet == 0)goto failed;

    /*音声*/
    hMenuAudio = ::CreateMenu();
    if (hMenuAudio == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioLoop, "Loop");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioSetting, "Setting");
    if (iRet == 0)goto failed;

    /*動画*/
    hMenuVideo = ::CreateMenu();
    if (hMenuVideo == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuVideo, MF_STRING, Menu::kVideoPause, "Pause");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuVideo, MF_STRING, Menu::kVideoSetting, "Setting");
    if (iRet == 0)goto failed;

    /*区分*/
    hMenuBar = ::CreateMenu();
    if (hMenuBar == nullptr) goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFolder), "Folder");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuAudio), "Audio");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuVideo), "Video");
    if (iRet == 0)goto failed;

    iRet = ::SetMenu(m_hWnd, hMenuBar);
    if (iRet == 0)goto failed;

    m_hMenuBar = hMenuBar;

    /*正常終了*/
    return;

failed:
    std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
    ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    /*SetMenu成功後はウィンドウ破棄時に破棄されるが、今は紐づけ前なのでここで破棄する。*/
    if (hMenuFolder != nullptr)
    {
        ::DestroyMenu(hMenuFolder);
    }
    if (hMenuAudio != nullptr)
    {
        ::DestroyMenu(hMenuAudio);
    }
    if (hMenuVideo != nullptr)
    {
        ::DestroyMenu(hMenuVideo);
    }
    if (hMenuBar != nullptr)
    {
        ::DestroyMenu(hMenuBar);
    }

}
/*フォルダ選択*/
void CMainWindow::MenuOnOpenFolder()
{
    std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(m_hWnd);
    if (!wstrPickedFolder.empty())
    {
        SetupScenario(wstrPickedFolder.c_str());
        CreateFolderList(wstrPickedFolder.c_str());
    }
}
/*音声ループ設定変更*/
void CMainWindow::MenuOnAudioLoop()
{
    if (m_pAudioPlayer != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kAudio);
            if (hMenu != nullptr)
            {
                BOOL iRet = m_pAudioPlayer->SwitchLoop();
                ::CheckMenuItem(hMenu, Menu::kAudioLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*音声設定画面呼び出し*/
void CMainWindow::MenuOnAudioSetting()
{
    if (m_pAudioPlayer != nullptr)
    {
        CMediaSettingDialogue* pMediaSettingDialogue = new CMediaSettingDialogue();
        if (pMediaSettingDialogue != nullptr)
        {
            pMediaSettingDialogue->Open(m_hInstance, m_hWnd, m_pAudioPlayer, L"Audio");

            delete pMediaSettingDialogue;
        }
    }
}
/*動画一時停止*/
void CMainWindow::MenuOnVideoPause()
{
    if (m_pVideoTransferor != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kVideo);
            if (hMenu != nullptr)
            {
                BOOL iRet = m_pVideoTransferor->SwitchPause();
                ::CheckMenuItem(hMenu, Menu::kVideoPause, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*動画設定画面呼び出し*/
void CMainWindow::MenuOnVideoSetting()
{
    if (m_pVideoTransferor != nullptr)
    {
        CMediaSettingDialogue* pMediaSettingDialogue = new CMediaSettingDialogue();
        if (pMediaSettingDialogue != nullptr)
        {
            pMediaSettingDialogue->Open(m_hInstance, m_hWnd, m_pVideoTransferor, L"Video");

            delete pMediaSettingDialogue;
        }
    }
}
/*次フォルダに移動*/
void CMainWindow::KeyUpOnNextFolder()
{
    if (m_folders.empty())return;

    ++m_nFolderIndex;
    if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;
    SetupScenario(m_folders.at(m_nFolderIndex).c_str());
}
/*前フォルダに移動*/
void CMainWindow::KeyUpOnForeFolder()
{
    if (m_folders.empty())return;

    --m_nFolderIndex;
    if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;
    SetupScenario(m_folders.at(m_nFolderIndex).c_str());
}
/*標題変更*/
void CMainWindow::ChangeWindowTitle(const wchar_t* pzTitle)
{
    std::wstring wstr;
    if (pzTitle != nullptr)
    {
        std::wstring wstrTitle = pzTitle;
        size_t pos = wstrTitle.find_last_of(L"\\/");
        wstr = pos == std::wstring::npos ? wstrTitle : wstrTitle.substr(pos + 1);
    }

    ::SetWindowTextW(m_hWnd, wstr.empty() ? m_window_name.c_str() : wstr.c_str());
}
/*表示形式変更*/
void CMainWindow::SwitchWindowMode()
{
    if (!m_bPlayReady)return;

    LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    m_bBarHidden ^= true;

    if (m_bBarHidden)
    {
        RECT rect;
        ::GetWindowRect(m_hWnd, &rect);

        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
        ::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
        ::SetMenu(m_hWnd, nullptr);
    }
    else
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
        ::SetMenu(m_hWnd, m_hMenuBar);
    }

    if (m_pViewManager != nullptr)
    {
        m_pViewManager->OnStyleChanged();
    }
}
/*フォルダ一覧表作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return false;

    m_folders.clear();
    m_nFolderIndex = 0;
    win_filesystem::GetFilePathListAndIndex(pwzFolderPath, nullptr, m_folders, &m_nFolderIndex);

    return m_folders.size() > 0;
}
/*寸劇構築*/
void CMainWindow::SetupScenario(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return;

    EndThreadpoolTimer();
    ClearScenarioInfo();
    ClearStoeredVideoFrame();

    m_bHasVideo = win_filesystem::CreateFilePathList(pwzFolderPath, L".mp4", m_videoFilePaths);

    std::vector<std::wstring> imageFilePaths;
    bool bHasImage = win_filesystem::CreateFilePathList(pwzFolderPath, L".jpg", imageFilePaths);
    if (bHasImage)
    {
        for (const auto& imageFilePath : imageFilePaths)
        {
            SImageFrame s{};
            bool bRet = win_image::LoadImageToMemory(imageFilePath.c_str(), &s, 1.f);
            if (bRet)
            {
                m_imageFrames.push_back(s);
            }
        }
    }

    std::vector<std::wstring> textFile;
    win_filesystem::CreateFilePathList(pwzFolderPath, L".txt", textFile);
    if (!textFile.empty())
    {
        tnfr::LoadScenario(textFile[0], m_textData);

        /*.txt無し、或いは読み取り失敗*/
        if (m_textData.empty())
        {
            std::vector<std::wstring> audioFilePaths;
            win_filesystem::CreateFilePathList(pwzFolderPath, L".mp3", audioFilePaths);
            for (const std::wstring& audioFilePath : audioFilePaths)
            {
                m_textData.emplace_back(adv::TextDatum{ L"", audioFilePath });
            }
        }
    }

    if (m_bHasVideo)
    {
        StartVideoPlaying();
    }
    else if (!m_bHasVideo && bHasImage)
    {
        if (!m_imageFrames.empty() && m_pViewManager != nullptr)
        {
            const SImageFrame& s = m_imageFrames[0];
            m_pViewManager->SetBaseSize(s.uiWidth, s.uiHeight);
            m_pViewManager->ResetZoom();
        }
    }

    UpdateText();

    m_bPlayReady = m_bHasVideo || bHasImage;

    ChangeWindowTitle(m_bPlayReady ? pwzFolderPath : nullptr);
}
/*寸劇情報消去*/
void CMainWindow::ClearScenarioInfo()
{
    m_textData.clear();
    m_nTextIndex = 0;

    m_imageFrames.clear();
    m_nImageIndex = 0;

    m_videoFilePaths.clear();
    m_nVideoIndex = 0;

    m_bFirstVideoLoaded = false;
}
/*再描画要求*/
void CMainWindow::UpdateScreen()
{
    ::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*表示画像送り・戻し*/
void CMainWindow::ShiftImage(bool bForward)
{
    if (m_bHasVideo)
    {
        if (bForward)
        {
            ++m_nVideoIndex;
            if (m_nVideoIndex >= m_videoFilePaths.size())m_nVideoIndex = 0;
        }
        else
        {
            --m_nVideoIndex;
            if (m_nVideoIndex >= m_videoFilePaths.size())m_nVideoIndex = m_videoFilePaths.size() - 1;
        }
        ClearStoeredVideoFrame();
        StartVideoPlaying();
    }
    else
    {
        if (bForward)
        {
            ++m_nImageIndex;
            if (m_nImageIndex >= m_imageFrames.size())m_nImageIndex = 0;
        }
        else
        {
            --m_nImageIndex;
            if (m_nImageIndex >= m_imageFrames.size())m_nImageIndex = m_imageFrames.size() - 1;
        }

        UpdateScreen();
    }
}
/*文章送り・戻し*/
void CMainWindow::ShiftText(bool bForward)
{
    if (bForward)
    {
        ++m_nTextIndex;
        if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
    }
    else
    {
        --m_nTextIndex;
        if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
    }
    UpdateText();
}
/*文章更新*/
void CMainWindow::UpdateText()
{
    if (m_nTextIndex < m_textData.size())
    {
        const adv::TextDatum& t = m_textData.at(m_nTextIndex);
        if (!t.wstrVoicePath.empty())
        {
            if (m_pAudioPlayer != nullptr)
            {
                m_pAudioPlayer->Play(t.wstrVoicePath.c_str());
            }
        }
        constexpr unsigned int kTimerInterval = 2000;
        ::SetTimer(m_hWnd, Timer::kText, kTimerInterval, nullptr);
    }

    UpdateScreen();
}
/*自動送り*/
void CMainWindow::AutoTexting()
{
    if (m_nTextIndex < m_textData.size() - 1)ShiftText(true);
}
/*表示文作成*/
std::wstring CMainWindow::FormatCurrentText()
{
    if (m_nTextIndex > m_textData.size() - 1)return std::wstring();

    const adv::TextDatum& t = m_textData.at(m_nTextIndex);
    std::wstring wstr = t.wstrText;
    if (!wstr.empty() && wstr.back() != L'\n')wstr.push_back(L'\n');
    wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
    return wstr;
}

/*転送動画溜め置き*/
void CMainWindow::StoreVideoFrame(long long llCurrentTime, const SImageFrame& imageFrame)
{
    constexpr int kMaxBufferMilliSeconds = 200;
    if (llCurrentTime < kMaxBufferMilliSeconds)
    {
        m_storedVideoFrames.insert({ llCurrentTime, imageFrame });
    }
}
/*溜め置き転送動画取り出し*/
SImageFrame* CMainWindow::ReStoreVideoFrame(long long llCurrentTime)
{
    const auto iter = m_storedVideoFrames.find(llCurrentTime);
    if (iter != m_storedVideoFrames.cend())
    {
        return &iter->second;
    }
    return nullptr;
}
/*一時保存方框消去*/
void CMainWindow::ClearStoeredVideoFrame()
{
    m_storedVideoFrames.clear();
}
/*動画再生開始*/
void CMainWindow::StartVideoPlaying()
{
    if (m_pVideoTransferor != nullptr && m_bHasVideo && m_nVideoIndex < m_videoFilePaths.size())
    {
        m_pVideoTransferor->Play(m_videoFilePaths.at(m_nVideoIndex).c_str());
        StartThreadpoolTimer();
    }
}
/*IMFMediaEngineNotify::EventNotify*/
void CMainWindow::OnAudioPlayerEvent(unsigned long ulEvent)
{
    switch (ulEvent)
    {
    case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:

        break;
    case MF_MEDIA_ENGINE_EVENT_ENDED:
        AutoTexting();
        break;
    default:
        break;
    }
}

void CMainWindow::OnVideoPlayerEvent(unsigned long ulEvent)
{
    switch (ulEvent)
    {
    case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
        if (m_pVideoTransferor != nullptr)
        {
            if (!m_bFirstVideoLoaded)
            {
                unsigned long uiWidth = 0;
                unsigned long uiHeight = 0;
                bool bRet = m_pVideoTransferor->GetVideoSize(&uiWidth, &uiHeight);
                if (bRet)
                {
                    if (m_pViewManager != nullptr)
                    {
                        m_pViewManager->SetBaseSize(uiWidth, uiHeight);
                        m_pViewManager->ResetZoom(1 / 1.5f);
                    }
                    m_bFirstVideoLoaded = true;
                }

            }
        }
        break;
    case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:

        break;
    case MF_MEDIA_ENGINE_EVENT_ENDED:

        break;
    default:
        break;
    }
}
/*描画間隔設定*/
void CMainWindow::SetupDrawingInterval()
{
    DEVMODE sDevMode{};
    ::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &sDevMode);
    m_nInterval = 1000 / sDevMode.dmDisplayFrequency;
}
/*タイマ開始*/
void CMainWindow::StartThreadpoolTimer()
{
    if (m_pTpTimer != nullptr)return;

    m_pTpTimer = ::CreateThreadpoolTimer(TimerCallback, this, nullptr);
    if (m_pTpTimer != nullptr)
    {
        UpdateTimerInterval(m_pTpTimer);
    }
}
/*タイマ停止*/
void CMainWindow::EndThreadpoolTimer()
{
    if (m_pTpTimer != nullptr)
    {
        ::SetThreadpoolTimer(m_pTpTimer, nullptr, 0, 0);
        ::WaitForThreadpoolTimerCallbacks(m_pTpTimer, TRUE);
        ::CloseThreadpoolTimer(m_pTpTimer);
        m_pTpTimer = nullptr;
    }
}
/*呼び出し間隔更新*/
void CMainWindow::UpdateTimerInterval(PTP_TIMER timer)
{
    if (timer != nullptr)
    {
        FILETIME sFileDueTime{};
        ULARGE_INTEGER ulDueTime{};
        ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * m_nInterval));
        sFileDueTime.dwHighDateTime = ulDueTime.HighPart;
        sFileDueTime.dwLowDateTime = ulDueTime.LowPart;
        ::SetThreadpoolTimer(timer, &sFileDueTime, 0, 0);
    }
}
/*満了*/
void CMainWindow::OnTide()
{
    UpdateScreen();
}
/*PTP_TIMER_CALLBACK*/
void CMainWindow::TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
    CMainWindow* pThis = static_cast<CMainWindow*>(Context);
    if (pThis != nullptr)
    {
        pThis->OnTide();
        pThis->UpdateTimerInterval(Timer);
    }
}