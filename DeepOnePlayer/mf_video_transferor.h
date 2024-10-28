#ifndef MF_VIDEO_TRANSFEROR_H_
#define MF_VIDEO_TRANSFEROR_H_

/*
* MF Media Engine in frame-server mode.
* 
* For fundamental features, rendering mode is easier to use.
* But in rendering mode, the video frame is always drawn on topmost of the target window that
* it is impossible to draw text on video frame.
* Thus, frame-server mode is used to controll drawring order.
* 
*/

#include <wincodec.h>

#include "mf_media_player.h"
#include "image_frame.h"

class CMfVideoTransferor : public CMfMediaPlayer
{
public:
	CMfVideoTransferor();
	~CMfVideoTransferor();

	bool TransferVideoFrame(SImageFrame* pImageFrame, long long* llCurrentTime = nullptr);

	bool SetPlaybackWindow(HWND hWnd, UINT uMsg);
	bool ResizeBuffer();
private:
	IWICBitmap *m_pWicBitmap = nullptr;

	void ReleaseWicBitmap();
	bool CreateWicBitmap(unsigned long uiWidth, unsigned long uiHeight);
	bool CheckWicBitmapSize(unsigned long uiWidth, unsigned long uiHeight);
};

#endif // !MF_VIDEO_TRANSFEROR_H_
