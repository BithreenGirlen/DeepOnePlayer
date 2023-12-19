
#include <Windows.h>
#include <CommCtrl.h>


#include "main_window.h"
#include "file_operation.h"
#include "file_system_utility.h"
#include "media_setting_dialogue.h"
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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_DEEPONE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_ICON_DEEPONE);
    wcex.lpszClassName = m_class_name.c_str();
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_DEEPONE));

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
        return m_bHasVideo ? ::DefWindowProcW(hWnd, uMsg, wParam, lParam) : 0;
    case WM_COMMAND:
        return OnCommand(wParam);
    case WM_MOUSEWHEEL:
        return OnMouseWheel(wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(wParam, lParam);
    case WM_MBUTTONUP:
        return OnMButtonUp(wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    InitialiseMenuBar();

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    ::PostQuitMessage(0);

    if (m_pScenePlayer != nullptr)
    {
        delete m_pScenePlayer;
        m_pScenePlayer = nullptr;
    }

    if (m_pVideoPlayer != nullptr)
    {
        delete m_pVideoPlayer;
        m_pVideoPlayer = nullptr;
    }

    if (m_pAudioPlayer != nullptr)
    {
        delete m_pAudioPlayer;
        m_pAudioPlayer = nullptr;
    }

    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_class_name.c_str(), m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    if (m_pScenePlayer != nullptr && !m_bHasVideo)
    {
        m_pScenePlayer->DisplayImage();
    }

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{

    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam)
{
    int wmKind = HIWORD(wParam);
    int wmId = LOWORD(wParam);
    if (wmKind == 0)
    {
        /*Menus*/
        switch (wmId)
        {
        case Menu::kOpenFolder:
            MenuOnOpenFolder();
            break;
        case Menu::kNextFolder:
            MenuOnNextFolder();
            break;
        case Menu::kForeFolder:
            MenuOnForeFolder();
            break;
        case Menu::kNextAudio:
            MenuOnNextAudio();
            break;
        case Menu::kForeAudio:
            MenuOnForeAudio();
            break;
        case Menu::kPlayAudio:
            MenuOnPlayAudio();
            break;
        case Menu::kAudioLoop:
            MenuOnAudioLoop();
            break;
        case Menu::kAudioSetting:
            MenuOnAudioVolume();
            break;
        case Menu::kNextVideo:
            MenuOnNextVideo();
            break;
        case Menu::kForeVideo:
            MenuOnForeVideo();
            break;
        case Menu::kPlayVideo:
            MenuOnPlayVideo();
            break;
        case Menu::kVideoLoop:
            MenuOnVideoLoop();
            break;
        case Menu::kVideoSetting:
            MenuOnVideoVolume();
            break;
        }
    }
    if (wmKind > 1)
    {
        /*Controls*/
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
        if (m_pScenePlayer != nullptr && !m_bHasVideo)
        {
            if (iScroll > 0)
            {
                m_pScenePlayer->UpScale();
            }
            else
            {
                m_pScenePlayer->DownScale();
            }
        }

        if (m_pVideoPlayer != nullptr && m_bHasVideo)
        {
            if (iScroll > 0)
            {
                m_pVideoPlayer->UpScale();
            }
            else
            {
                m_pVideoPlayer->DownScale();
            }
        }

    }

    if (wKey == MK_RBUTTON && m_pAudioPlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pAudioPlayer->Next();
        }
        else
        {
            m_pAudioPlayer->Back();
        }
    }

    return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    ::GetCursorPos(&m_CursorPos);

    return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);

    if (usKey == MK_RBUTTON && m_bHideBar)
    {
        ::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_DOWN;
        ::SendInput(1, &input, sizeof(input));
    }

    if (usKey == 0)
    {
        POINT pt{};
        ::GetCursorPos(&pt);
        int iX = m_CursorPos.x - pt.x;
        int iY = m_CursorPos.y - pt.y;

        if (m_pScenePlayer != nullptr)
        {

            if (iX == 0 && iY == 0)
            {
                m_pScenePlayer->Next();
            }
            else
            {
                m_pScenePlayer->SetOffset(iX, iY);
            }
        }

        if (m_pVideoPlayer != nullptr && m_bHasVideo)
        {
            if (iX == 0 && iY == 0)
            {
                m_pVideoPlayer->Next();
            }
            else
            {
                ::ScreenToClient(m_hWnd, &pt);
                ::ScreenToClient(m_hWnd, &m_CursorPos);

                RECT rc{};
                rc.left = m_CursorPos.x > pt.x ? pt.x : m_CursorPos.x;
                rc.right = m_CursorPos.x > pt.x ? m_CursorPos.x : pt.x;
                rc.top = m_CursorPos.y > pt.y ? pt.y : m_CursorPos.y;
                rc.bottom = m_CursorPos.y > pt.y ? m_CursorPos.y : pt.y;

                m_pVideoPlayer->SetDisplayArea(&rc);
            }

        }

    }
    return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);
    if (usKey == 0)
    {
        if (m_pScenePlayer != nullptr && !m_bHasVideo)
        {
            m_pScenePlayer->ResetScale();
        }

        if (m_pVideoPlayer != nullptr && m_bHasVideo)
        {
            m_pVideoPlayer->ResetZoom();
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
    HMENU hManuFolder = nullptr;
    HMENU hMenuAudio = nullptr;
    HMENU hMenuVideo = nullptr;
    HMENU hMenuBar = nullptr;
    BOOL iRet = FALSE;

    if (m_hMenuBar != nullptr)return;

    /*フォルダ*/
    hManuFolder = ::CreateMenu();
    if (hManuFolder == nullptr)goto failed;
    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kOpenFolder, "Open");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kNextFolder, "Next");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kForeFolder, "Back");
    if (iRet == 0)goto failed;

    /*音声*/
    hMenuAudio = ::CreateMenu();
    if (hMenuAudio == nullptr)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kNextAudio, "Next");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kForeAudio, "Back");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kPlayAudio, "Play");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioLoop, "Loop");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioSetting, "Setting");
    if (iRet == 0)goto failed;

    /*動画*/
    hMenuVideo = ::CreateMenu();
    if (hMenuVideo == nullptr)goto failed;
    iRet = ::AppendMenuA(hMenuVideo, MF_STRING, Menu::kNextVideo, "Next");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuVideo, MF_STRING, Menu::kForeVideo, "Back");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuVideo, MF_STRING, Menu::kPlayVideo, "Play");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuVideo, MF_STRING, Menu::kVideoLoop, "Loop");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuVideo, MF_STRING, Menu::kVideoSetting, "Setting");
    if (iRet == 0)goto failed;

    /*上部欄*/
    hMenuBar = ::CreateMenu();
    if (hMenuBar == nullptr) goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hManuFolder), "Folder");
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
    if (hManuFolder != nullptr)
    {
        ::DestroyMenu(hManuFolder);
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
/*再生機初期化*/
void CMainWindow::InitialisePlayers()
{
    double dbVideoRate = 1.0;
    double dbVideoVolume = 0.5;
    double dbAudioRate = 1.0;
    double dbAudioVolume = 0.5;

    if (m_pScenePlayer != nullptr)
    {
        delete m_pScenePlayer;
        m_pScenePlayer = nullptr;
    }

    if (m_pVideoPlayer != nullptr)
    {
        dbVideoRate = m_pVideoPlayer->GetCurrentRate();
        dbVideoVolume = m_pVideoPlayer->GetCurrentVolume();
        delete m_pVideoPlayer;
        m_pVideoPlayer = nullptr;
    }

    if (m_pAudioPlayer != nullptr)
    {
        dbAudioRate = m_pAudioPlayer->GetCurrentRate();
        dbAudioVolume = m_pAudioPlayer->GetCurrentVolume();
        delete m_pAudioPlayer;
        m_pAudioPlayer = nullptr;
    }

    m_pScenePlayer = new CScenePlayer(m_hWnd);

    m_pVideoPlayer = new CMediaPlayer(m_hWnd);

    m_pAudioPlayer = new CMediaPlayer(nullptr);

    HMENU hMenuBar = ::GetMenu(m_hWnd);

    /*動画設定復旧*/
    if (m_pVideoPlayer != nullptr && hMenuBar != nullptr)
    {
        m_pVideoPlayer->SetCurrentRate(dbVideoRate);
        m_pVideoPlayer->SetCurrentVolume(dbVideoVolume);

        HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kVideo);
        if (hMenu != nullptr)
        {
            UINT uiState = ::GetMenuState(hMenu, Menu::kVideoLoop, MF_BYCOMMAND);
            if (uiState != -1)
            {
                /*初期状態では無効のはず*/
                if (uiState & MF_CHECKED)
                {
                    BOOL iRet = m_pVideoPlayer->SwitchLoop();
                    ::CheckMenuItem(hMenu, Menu::kVideoLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
                }
            }
        }
    }

    /*音声設定復旧*/
    if (m_pAudioPlayer != nullptr && hMenuBar != nullptr)
    {
        m_pAudioPlayer->SetCurrentRate(dbAudioRate);
        m_pAudioPlayer->SetCurrentVolume(dbAudioVolume);

        HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kAudio);
        if (hMenu != nullptr)
        {
            UINT uiState = ::GetMenuState(hMenu, Menu::kAudioLoop, MF_BYCOMMAND);
            if (uiState != -1)
            {
                if (uiState & MF_CHECKED)
                {
                    BOOL iRet = m_pAudioPlayer->SwitchLoop();
                    ::CheckMenuItem(hMenu, Menu::kAudioLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
                }
            }
        }
    }
    
}
/*フォルダ選択*/
void CMainWindow::MenuOnOpenFolder()
{
    wchar_t* buffer = SelectWorkingFolder(m_hWnd);
    if (buffer != nullptr)
    {
        SetPlayerFolder(buffer);
        CreateFolderList(buffer);

        ::CoTaskMemFree(buffer);
    }
}
/*次フォルダに移動*/
void CMainWindow::MenuOnNextFolder()
{
    if (m_folders.empty())return;

    ++m_nIndex;
    if (m_nIndex >= m_folders.size())m_nIndex = 0;
    SetPlayerFolder(m_folders.at(m_nIndex).c_str());
}
/*前フォルダに移動*/
void CMainWindow::MenuOnForeFolder()
{
    if (m_folders.empty())return;

    --m_nIndex;
    if (m_nIndex >= m_folders.size())m_nIndex = m_folders.size() - 1;
    SetPlayerFolder(m_folders.at(m_nIndex).c_str());
}
/*次音声再生*/
void CMainWindow::MenuOnNextAudio()
{
    if (m_pAudioPlayer != nullptr)
    {
        m_pAudioPlayer->Next();
    }
}
/*前音声再生*/
void CMainWindow::MenuOnForeAudio()
{
    if (m_pAudioPlayer != nullptr)
    {
        m_pAudioPlayer->Back();
    }
}
/*現行音声再再生*/
void CMainWindow::MenuOnPlayAudio()
{
    if (m_pAudioPlayer != nullptr)
    {
        m_pAudioPlayer->Play();
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
/*音量・再生速度変更*/
void CMainWindow::MenuOnAudioVolume()
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

void CMainWindow::MenuOnNextVideo()
{
    if (m_pVideoPlayer != nullptr)
    {
        m_pVideoPlayer->Next();
    }
}

void CMainWindow::MenuOnForeVideo()
{
    if (m_pVideoPlayer != nullptr)
    {
        m_pVideoPlayer->Back();
    }
}

void CMainWindow::MenuOnPlayVideo()
{
    if (m_pVideoPlayer != nullptr)
    {
        m_pVideoPlayer->Play();
    }
}

void CMainWindow::MenuOnVideoLoop()
{
    if (m_pVideoPlayer != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kVideo);
            if (hMenu != nullptr)
            {
                BOOL iRet = m_pVideoPlayer->SwitchLoop();
                ::CheckMenuItem(hMenu, Menu::kVideoLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}

void CMainWindow::MenuOnVideoVolume()
{
    if (m_pVideoPlayer != nullptr)
    {
        CMediaSettingDialogue* pMediaSettingDialogue = new CMediaSettingDialogue();
        if (pMediaSettingDialogue != nullptr)
        {
            pMediaSettingDialogue->Open(m_hInstance, m_hWnd, m_pVideoPlayer, L"Video");

            delete pMediaSettingDialogue;
        }
    }
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

    m_bHideBar ^= true;

    if (m_bHideBar)
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

    if (m_pScenePlayer != nullptr)
    {
        m_pScenePlayer->SwitchSizeLore(m_bHideBar);
    }

    if (m_pVideoPlayer != nullptr)
    {
        m_pVideoPlayer->SwitchSizeLore(m_bHideBar);
    }
}
/*フォルダ一覧表作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return false;

    m_folders.clear();
    m_nIndex = 0;

    std::wstring wstrFolder = pwzFolderPath;
    std::wstring wstrParent;
    std::wstring wstrCurrent;

    size_t nPos = wstrFolder.find_last_of(L"\\/");
    if (nPos != std::wstring::npos)
    {
        wstrParent = wstrFolder.substr(0, nPos);
        wstrCurrent = wstrFolder.substr(nPos + 1);
    }

    if (wstrParent.empty())return false;

    CreateFilePathList(wstrParent.c_str(), nullptr, m_folders);

    auto iter = std::find(m_folders.begin(), m_folders.end(), wstrFolder);
    if (iter != m_folders.end())
    {
        m_nIndex = std::distance(m_folders.begin(), iter);
    }

    return m_folders.size() > 0;
}
/*再生フォルダ設定*/
void CMainWindow::SetPlayerFolder(const wchar_t* pwzFolderPath)
{
    InitialisePlayers();

    bool bRet = false;

    if (m_pAudioPlayer != nullptr)
    {
        std::vector<std::wstring> textFile;
        CreateFilePathList(pwzFolderPath, L".txt", textFile);
        if (!textFile.empty())
        {
            std::vector<std::wstring> audioFileNames;
            FindAudioFileNames(textFile.at(0).c_str(), audioFileNames);
            if (!audioFileNames.empty())
            {
                std::vector<std::wstring> filePaths;
                for (const std::wstring& audioFileName : audioFileNames)
                {
                    filePaths.push_back(std::wstring(pwzFolderPath).append(L"\\").append(audioFileName));
                }
                m_pAudioPlayer->SetFiles(filePaths);
            }
        }
        else
        {
            std::vector<std::wstring> filePaths;
            bRet = CreateFilePathList(pwzFolderPath, L".mp3", filePaths);
            if (bRet)
            {
                m_pAudioPlayer->SetFiles(filePaths);
            }
        }
    }

    m_bHasVideo = false;

    if (m_pVideoPlayer != nullptr)
    {
        std::vector<std::wstring> filePaths;
        bRet = CreateFilePathList(pwzFolderPath, L".mp4", filePaths);
        if (bRet)
        {
            m_bHasVideo = m_pVideoPlayer->SetFiles(filePaths);
        }
    }

    bool bHasImage = false;

    if (m_pScenePlayer != nullptr && !m_bHasVideo)
    {
        std::vector<std::wstring> filePaths;
        bRet = CreateFilePathList(pwzFolderPath, L".jpg", filePaths);
        if (bRet)
        {
            bHasImage = m_pScenePlayer->SetFiles(filePaths);
        }
    }

    m_bPlayReady = m_bHasVideo || bHasImage;

    ChangeWindowTitle(m_bPlayReady ? pwzFolderPath : nullptr);
}
/*脚本内音声ファイル名探索*/
void CMainWindow::FindAudioFileNames(const wchar_t* pwzFilePath, std::vector<std::wstring>& names)
{
    std::wstring wstrText = LoadFileAsString(pwzFilePath);

    const wchar_t key[] = L"playvoice,1,";

    size_t nRead = 0;
    size_t nPos = 0;
    size_t nEnd = 0;

    std::vector<std::wstring> audioFilePaths;

    for (;;)
    {
        nPos = wstrText.substr(nRead).find(key);
        if (nPos == std::wstring::npos)break;

        nRead += nPos + sizeof(key)-1;
        nEnd = wstrText.substr(nRead).find_first_of(L",\r\n");
        if (nEnd == std::wstring::npos)break;

        audioFilePaths.push_back(wstrText.substr(nRead, nEnd));
        nRead += nEnd;
    }

    for (const std::wstring &audioFilePath : audioFilePaths)
    {
        nPos = audioFilePath.rfind(L"/");
        if (nPos == std::wstring::npos)continue;
        names.push_back(audioFilePath.substr(nPos + 1));
    }

}
