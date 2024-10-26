#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <unordered_map>

#include "d2_image_drawer.h"
#include "d2_text_writer.h"
#include "mf_media_player.h"
#include "mf_video_transferor.h"
#include "view_manager.h"
#include "adv.h"

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
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu
	{
		kOpenFolder = 1, kNextFolder, kForeFolder,
		kAudioLoop, kAudioSetting,
		kVideoPause, kVideoSetting
	};
	enum MenuBar
	{
		kFolder, kAudio, kVideo
	};
	enum EventMessage
	{
		kAudioPlayer = WM_USER + 1,
		kVideoPlayer
	};
	enum Timer
	{
		kText = 1,
	};

	POINT m_CursorPos{};
	bool m_bLeftDowned = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;
	bool m_bPlayReady = false;
	bool m_bHasVideo = false;
	bool m_bTextHidden = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	void InitialiseMenuBar();

	void MenuOnOpenFolder();
	void MenuOnNextFolder();
	void MenuOnForeFolder();

	void MenuOnAudioLoop();
	void MenuOnAudioSetting();

	void MenuOnVideoPause();
	void MenuOnVideoSetting();

	void ChangeWindowTitle(const wchar_t* pzTitle);
	void SwitchWindowMode();

	bool CreateFolderList(const wchar_t* pwzFolderPath);
	void SetupScenario(const wchar_t* pwzFolderPath);
	void ClearScenarioInfo();

	void UpdateScreen();

	CD2ImageDrawer* m_pD2ImageDrawer = nullptr;
	CD2TextWriter* m_pD2TextWriter = nullptr;
	CMfMediaPlayer* m_pAudioPlayer = nullptr;
	CMfVideoTransferor* m_pVideoTransferor = nullptr;
	CViewManager* m_pViewManager = nullptr;

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;

	std::vector<SImageFrame> m_imageFrames;
	size_t m_nImageIndex = 0;

	std::vector<std::wstring> m_videoFilePaths;
	size_t m_nVideoIndex = 0;

	bool m_bFirstVideoLoaded = false;

	void ShiftImage(bool bForward);
	void ShiftText(bool bForward);
	void UpdateText();
	void AutoTexting();
	std::wstring FormatCurrentText();

	std::unordered_map<long long, SImageFrame> m_storedVideoFrames;
	void StoreVideoFrame(long long llCurrentTime, const SImageFrame& imageFrame);
	SImageFrame* ReStoreVideoFrame(long long llCurrentTime);
	void ClearStoeredVideoFrame();

	void StartVideoPlaying();

	void OnAudioPlayerEvent(unsigned long ulEvent);
	void OnVideoPlayerEvent(unsigned long ulEvent);

	long long m_nInterval = 16;
	PTP_TIMER m_pTpTimer = nullptr;

	void SetupDrawingInterval();

	void StartThreadpoolTimer();
	void EndThreadpoolTimer();
	void UpdateTimerInterval(PTP_TIMER timer);
	void OnTide();
	static void CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer);
};

#endif //MAIN_WINDOW_H_