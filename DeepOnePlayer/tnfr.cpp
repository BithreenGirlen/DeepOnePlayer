

#include "tnfr.h"
#include "win_filesystem.h"
#include "win_text.h"

namespace tnfr
{
    void TextToLines(const std::wstring& wstrText, std::vector<std::wstring>& lines)
    {
        std::wstring wstrTemp;
        for (size_t nRead = 0; nRead < wstrText.size(); ++nRead)
        {
            if (wstrText.at(nRead) == L'\r' || wstrText.at(nRead) == L'\n')
            {
                if (!wstrTemp.empty())
                {
                    lines.push_back(wstrTemp);
                    wstrTemp.clear();
                }
                continue;
            }

            wstrTemp.push_back(wstrText.at(nRead));
        }

        if (!wstrTemp.empty())
        {
            lines.push_back(wstrTemp);
            wstrTemp.clear();
        }
    }

    void SplitTextBySeparator(const std::wstring& wstrText, const wchar_t cSeparator, std::vector<std::wstring>& splits)
    {
        for (size_t nRead = 0; nRead < wstrText.size();)
        {
            const wchar_t* p = wcschr(&wstrText[nRead], cSeparator);
            if (p == nullptr)
            {
                size_t nLen = wstrText.size() - nRead;
                splits.emplace_back(wstrText.substr(nRead, nLen));
                break;
            }

            size_t nLen = p - &wstrText[nRead];
            splits.emplace_back(wstrText.substr(nRead, nLen));
            nRead += nLen + 1;
        }
    }

    void ReplaceAll(std::wstring& src, const std::wstring& strOld, const std::wstring& strNew)
    {
        if (strOld == strNew)return;

        for (size_t nRead = 0;;)
        {
            size_t nPos = src.find(strOld, nRead);
            if (nPos == std::wstring::npos)break;
            src.replace(nPos, strOld.size(), strNew);
            nRead = nPos + strNew.size();
        }
    }

    void EliminateTag(std::wstring& wstr)
    {
        std::wstring wstrResult;
        wstrResult.reserve(wstr.size());
        int iCount = 0;
        for (const auto& c : wstr)
        {
            if (c == L'<')
            {
                ++iCount;
                continue;
            }
            else if (c == L'>')
            {
                --iCount;
                continue;
            }

            if (iCount == 0)
            {
                wstrResult.push_back(c);
            }
        }
        wstr = wstrResult;
    }


    std::wstring ExtractDirectory(const std::wstring& wstrFilePath)
    {
        size_t nPos = wstrFilePath.find_last_of(L"\\/");
        if (nPos != std::wstring::npos)
        {
            return wstrFilePath.substr(0, nPos);
        }
        return wstrFilePath;
    }
    std::wstring TruncateFilePath(const std::wstring& strRelativePath)
    {
        size_t nPos = strRelativePath.rfind(L'/');
        if (nPos != std::wstring::npos)
        {
            return strRelativePath.substr(nPos + 1);
        }
        return strRelativePath;
    }
}

bool tnfr::LoadScenario(const std::wstring &wstrFilePath, std::vector<adv::TextDatum>& textData)
{
    std::wstring wstrText = win_text::WidenUtf8(win_filesystem::LoadFileAsString(wstrFilePath.c_str()));

    std::wstring wstrParent = ExtractDirectory(wstrFilePath);

    std::vector<std::wstring> lines;
    TextToLines(wstrText, lines);

    std::wstring playVoiceBuffer;

    for (auto& line : lines)
    {
        std::vector<std::wstring> columns;
        SplitTextBySeparator(line, L',', columns);

        if (columns.empty())continue;

        const auto& strType = columns.at(0);
        for (auto& column : columns)ReplaceAll(column, L"\t", L"");

        if (strType == L"playvoice")
        {
            if (columns.size() > 2)
            {
                playVoiceBuffer = columns.at(2);
            }
        }
        else if (strType == L"msg")
        {
            if (columns.size() > 2 && !columns.at(2).empty())
            {
                adv::TextDatum t;
                EliminateTag(columns.at(2));
                ReplaceAll(columns.at(2), L"\\n", L"\n");
                t.wstrText = columns.at(2);
                if (!playVoiceBuffer.empty())
                {
                    t.wstrVoicePath = wstrParent + L"\\" + TruncateFilePath(playVoiceBuffer);
                    playVoiceBuffer.clear();
                }

                textData.push_back(t);
            }
        }
    }

    return !textData.empty();
}
