

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

    void EliminateRuby(std::wstring& wstr)
    {
        for (size_t nRead = 0;;)
        {
            size_t nPos = wstr.find(L"<ruby>", nRead);
            if (nPos == std::wstring::npos)break;

            size_t nPos1 = wstr.find(L'|', nPos);
            if (nPos1 == std::wstring::npos)break;

            size_t nPos2 = wstr.find(L'<', nPos1);
            if (nPos2 == std::wstring::npos)break;

            size_t nLen = nPos2 - nPos1;
            wstr.replace(nPos1, nLen, L"");
            nRead = nPos1;
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
        size_t nPos = strRelativePath.find_last_of(L"\\/");
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

    std::wstring nameBuffer;
    std::wstring playVoiceBuffer;

    for (auto& line : lines)
    {
        std::vector<std::wstring> columns;
        SplitTextBySeparator(line, L',', columns);

        if (columns.empty())continue;

        const auto& strType = columns[0];
        for (auto& column : columns)ReplaceAll(column, L"\t", L"");

        if (strType == L"name")
        {
            if (columns.size() > 1 && columns[1][0] == '<')
            {
                EliminateRuby(columns[1]);
                EliminateTag(columns[1]);
                nameBuffer = columns[1];
            }
            else
            {
                nameBuffer.clear();
            }
        }
        else if (strType == L"playvoice")
        {
            if (columns.size() > 2)
            {
                playVoiceBuffer = columns[2];
            }
        }
        else if (strType == L"msg")
        {
            if (columns.size() > 2 && !columns[2].empty())
            {
                std::wstring& wstr = columns[2];
                adv::TextDatum t;
                t.wstrText.reserve(128);
                EliminateTag(wstr);
                ReplaceAll(wstr, L"\\n", L"\n");

                if (!nameBuffer.empty())
                {
                    t.wstrText = nameBuffer;
                    t.wstrText += L": \n";
                }

                t.wstrText += wstr;
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
