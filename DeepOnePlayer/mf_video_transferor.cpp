
#include <atlbase.h>
#include <dxgiformat.h>

#include "mf_video_transferor.h"

#pragma comment (lib,"Windowscodecs.lib")

CMfVideoTransferor::CMfVideoTransferor()
{

}

CMfVideoTransferor::~CMfVideoTransferor()
{
	ReleaseWicBitmap();
}
/*“]‘—*/
bool CMfVideoTransferor::TransferVideoFrame(SImageFrame* pImageFrame, long long* llCurrentTime)
{
	if (pImageFrame == nullptr)return false;

	if (m_pmfEngineEx != nullptr)
	{
		BOOL iRet = m_pmfEngineEx->HasVideo();
		if (iRet)
		{
			long long llReadyFrame = 0;
			HRESULT hr = m_pmfEngineEx->OnVideoStreamTick(&llReadyFrame);
			if (SUCCEEDED(hr) && llReadyFrame >= 0)
			{
				if (llCurrentTime != nullptr)
				{
					*llCurrentTime = GetCurrentTimeInMilliSeconds();
				}

				unsigned long ulDestWidth = 0;
				unsigned long ulDestHeight = 0;
				bool bRet = GetVideoSize(&ulDestWidth, &ulDestHeight);
				if (!bRet)return false;

				bRet = CheckWicBitmapSize(ulDestWidth, ulDestHeight);
				if (!bRet)return false;

				RECT dstRect{};
				dstRect.right = ulDestWidth;
				dstRect.bottom = ulDestHeight;
				MFARGB bg{ 0, 0, 0, 0 };
				MFVideoNormalizedRect normalisedRect{};
				hr = m_pmfEngineEx->TransferVideoFrame(m_pWicBitmap, &normalisedRect, &dstRect, &bg);
				if (FAILED(hr))return false;

				unsigned int uiWidth = 0;
				unsigned int uiHeight = 0;
				hr = m_pWicBitmap->GetSize(&uiWidth, &uiHeight);
				if (FAILED(hr))return false;

				CComPtr<IWICBitmapLock> pWicBitmapLock;
				WICRect wicRect{ 0, 0, static_cast<INT>(uiWidth), static_cast<INT>(uiHeight) };
				hr = m_pWicBitmap->Lock(&wicRect, WICBitmapLockFlags::WICBitmapLockRead, &pWicBitmapLock);
				if (FAILED(hr))return false;

				unsigned int uiStride = 0;
				hr = pWicBitmapLock->GetStride(&uiStride);
				if (FAILED(hr))return false;

				pImageFrame->uiWidth = uiWidth;
				pImageFrame->uiHeight = uiHeight;
				pImageFrame->iStride = uiStride;

				size_t nPixelSize = static_cast<size_t>(uiStride * uiHeight);
				pImageFrame->pixels.resize(nPixelSize);

				hr = m_pWicBitmap->CopyPixels(nullptr, uiStride, static_cast<UINT>(pImageFrame->pixels.size()), pImageFrame->pixels.data());

				return SUCCEEDED(hr);
			}
		}
	}
	return false;
}

bool CMfVideoTransferor::SetPlaybackWindow(HWND hWnd, UINT uMsg)
{
	m_hRetWnd = hWnd;
	m_uRetMsg = uMsg;

	HRESULT hr = m_pMfAttributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM);
	return SUCCEEDED(hr);
}

bool CMfVideoTransferor::ResizeBuffer()
{
	/*rendering mode only*/
	return false;
}

void CMfVideoTransferor::ReleaseWicBitmap()
{
	if (m_pWicBitmap != nullptr)
	{
		m_pWicBitmap->Release();
		m_pWicBitmap = nullptr;
	}
}

bool CMfVideoTransferor::CreateWicBitmap(unsigned long uiWidth, unsigned long uiHeight)
{
	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX::CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	ReleaseWicBitmap();

	hr = pWicImageFactory->CreateBitmap(uiWidth, uiHeight, GUID_WICPixelFormat32bppPBGRA, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &m_pWicBitmap);

	return SUCCEEDED(hr);
}

bool CMfVideoTransferor::CheckWicBitmapSize(unsigned long uiWidth, unsigned long uiHeight)
{
	if (m_pWicBitmap == nullptr)
	{
		return CreateWicBitmap(uiWidth, uiHeight);
	}
	else
	{
		unsigned int uiCurrentWidth = 0;
		unsigned int uiCUrrenHeight = 0;
		m_pWicBitmap->GetSize(&uiCurrentWidth, &uiCUrrenHeight);
		if (uiWidth > uiCurrentWidth && uiHeight > uiCUrrenHeight)
		{
			return CreateWicBitmap(uiWidth, uiHeight);
		}
		else
		{
			return true;
		}
	}
	return false;
}
