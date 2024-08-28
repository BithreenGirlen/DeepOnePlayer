#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "scene_player.h"
#include "media_player.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	std::wstring m_class_name = L"DeepOne player window";
	std::wstring m_window_name = L"DeepOne player";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu{kOpenFolder = 1, kNextFolder, kForeFolder,
		kNextAudio, kForeAudio, kPlayAudio, kAudioLoop, kAudioSetting,
		kNextVideo, kForeVideo, kPlayVideo, kVideoLoop, kVideoSetting,};
	enum MenuBar{kFolder, kAudio, kVideo};
	POINT m_CursorPos{};

	HMENU m_hMenuBar = nullptr;
	bool m_bHideBar = false;
	bool m_bPlayReady = false;
	bool m_bHasVideo = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	void InitialiseMenuBar();
	void InitialisePlayers();

	void MenuOnOpenFolder();
	void MenuOnNextFolder();
	void MenuOnForeFolder();

	void MenuOnNextAudio();
	void MenuOnForeAudio();
	void MenuOnPlayAudio();
	void MenuOnAudioLoop();
	void MenuOnAudioVolume();

	void MenuOnNextVideo();
	void MenuOnForeVideo();
	void MenuOnPlayVideo();
	void MenuOnVideoLoop();
	void MenuOnVideoVolume();

	void ChangeWindowTitle(const wchar_t* pzTitle);
	void SwitchWindowMode();

	bool CreateFolderList(const wchar_t* pwzFolderPath);
	void SetPlayerFolder(const wchar_t* pwzFolderPath);
	void FindAudioFileNames(const wchar_t* pwzFilePath, std::vector<std::wstring> &names);

	CScenePlayer* m_pScenePlayer = nullptr;
	CMediaPlayer* m_pVideoPlayer = nullptr;
	CMediaPlayer* m_pAudioPlayer = nullptr;
};

#endif //MAIN_WINDOW_H_