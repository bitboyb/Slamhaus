#include "parser.hpp"
#include "element.hpp"
#include "id.hpp"
#include "seo.hpp"
#include "style.hpp"
#include "text.hpp"
#include "embed.hpp"
#include "snippet.hpp"
#include <sstream>
#include <cctype>

namespace Parser 
{
    std::string ParseMarkdown(const std::string &markdown)
    {
        std::istringstream iss(markdown);
        std::string line;
        std::ostringstream html;

        Parser::ParseState pState;
    
        std::ostringstream columnBuffer;
        std::vector<std::string> columnParts;
        std::vector<std::string> columnOpenLines;
    
        while (std::getline(iss, line))
        {
            if (SEO::IsSiteLine(line))
            {
                SEO::ProcessSiteLine(line);
                continue;
            }
            if (SEO::IsPageLine(line))
            {
                SEO::ProcessPageLine(line);
                continue;
            }
            if (Style::IsSectionOpenLine(line))
            {
                Text::CloseLists(html, pState);
                std::string htmlLine = "<section>";
                if (ID::isIDTag(line))
                {
                    htmlLine = ID::ApplyIDTag(htmlLine, line);
                }
                html << htmlLine << "\n";
                continue;
            }
            if (Style::IsSectionCloseLine(line))
            {
                html << "</section>\n";
                continue;
            }
            if (Style::IsColumnOpenLine(line))
            {
                if (pState.inColumn && !columnBuffer.str().empty())
                {
                    columnParts.push_back(columnBuffer.str());
                    columnOpenLines.push_back(line);
                    columnBuffer.str("");
                    columnBuffer.clear();
                }
                else if (!pState.inColumn)
                {
                    Text::CloseLists(html, pState);
                    pState.inColumn = true;
                    columnParts.clear();
                    columnOpenLines.clear();
                    columnBuffer.str("");
                    columnBuffer.clear();
                    columnOpenLines.push_back(line);
                }
                continue;
            }
            
            if (Style::IsColumnCloseLine(line))
            {
                if (!columnBuffer.str().empty())
                {
                    columnParts.push_back(columnBuffer.str());
                }
                for (auto &col : columnParts)
                {
                    col = Parser::ParseMarkdown(col);
                }
                html << Style::ApplyColumns(static_cast<int>(columnParts.size()), columnParts, columnOpenLines);
                pState.inColumn = false;
                columnBuffer.str("");
                columnBuffer.clear();
                continue;
            }
            if (pState.inColumn)
            {
                if (line == "---")
                {
                    columnParts.push_back(columnBuffer.str());
                    columnBuffer.str("");
                    columnBuffer.clear();
                }
                else
                {
                    columnBuffer << line << "\n";
                }
                continue;
            }
            if (Element::IsFormOpenLine(line))
            {

            }
            if (Element::IsFormCloseLine(line))
            {

            }
            if (Element::IsButtonLine(line))
            {
                Text::CloseLists(html, pState);
                html << Element::ProcessButton(line) << "\n";
                continue;
            }            
            if (Snippet::IsCodeBlockLine(line))
            {
                if (!pState.inCodeBlock)
                {
                    Text::CloseLists(html, pState);
                    pState.inCodeBlock = true;
                    html << Snippet::ParseCodeBlock(iss, line);
                    pState.inCodeBlock = false;
                }
                continue;
            }
            if (line.empty())
            {
                Text::CloseLists(html, pState);
                continue;
            }
            if (Embed::IsIFrameLine(line))
            {
                Text::CloseLists(html, pState);
                html << Embed::ProcessIFrame(line);
                continue;
            }
            if (Embed::IsAudioLine(line))
            {
                Text::CloseLists(html, pState);
                html << Embed::ProcessAudio(line);
                continue;
            }
            if (Embed::IsPictureLine(line))
            {
                Text::CloseLists(html, pState);
                html << Embed::ProcessPictures(line);
                continue;
            }
            if (Embed::IsSvgLine(line))
            {
                Text::CloseLists(html, pState);
                html << Embed::ProcessSvg(line);
                continue;
            }
            if (Text::IsHorizontalRuleLine(line))
            {
                Text::CloseLists(html, pState);
                html << "<hr>\n";
                continue;
            }
            if (Text::IsTableLine(line))
            {
                Text::CloseLists(html, pState);
                html << Text::ParseTable(iss, line);
                continue;
            }
            if (Embed::IsImageLine(line))
            {
                Text::CloseLists(html, pState);
                html << Embed::ProcessImages(line);
                continue;
            }
            if (Embed::IsVideoLine(line))
            {
                Text::CloseLists(html, pState);
                html << Embed::ProcessVideos(line);
                continue;
            }
            if (Text::IsHeading1(line))
            {
                Text::CloseLists(html, pState);
                Text::AppendHeading(html, 1, line.substr(2));
            }
            else if (Text::IsHeading2(line))
            {
                Text::CloseLists(html, pState);
                Text::AppendHeading(html, 2, line.substr(3));
            }
            else if (Text::IsHeading3(line))
            {
                Text::CloseLists(html, pState);
                Text::AppendHeading(html, 3, line.substr(4));
            }
            else if (Text::IsHeading4(line))
            {
                Text::CloseLists(html, pState);
                Text::AppendHeading(html, 4, line.substr(5));
            }
            else if (Text::IsUnorderedListLine(line))
            {
                if (!pState.inUl)
                {
                    Text::CloseLists(html, pState);
                    html << "<ul>\n";
                    pState.inUl = true;
                }
                Text::AppendListItem(html, line.substr(2));
            }
            else if (Text::IsNumberedListLine(line))
            {
                if (!pState.inOl)
                {
                    Text::CloseLists(html, pState);
                    html << "<ol>\n";
                    pState.inOl = true;
                }
                size_t pos = line.find(". ");
                if (pos != std::string::npos)
                {
                    Text::AppendListItem(html, line.substr(pos + 2));
                }
                else
                {
                    Text::AppendListItem(html, line);
                }
            }
            else
            {
                Text::CloseLists(html, pState);
                Text::AppendParagraph(html, line);
            }
        }
        Text::CloseLists(html, pState);
        return html.str();
    }    

    std::string StripParagraphTags(const std::string &html)
    {
        std::string result = html;
        if ((result.size() >= 7) && (result.substr(0, 3) == "<p>") && (result.substr(result.size() - 4) == "</p>"))
        {
            result = result.substr(3, result.size() - 7);
        }
        return result;
    }

    std::string ExtractSiteTitle(const std::string &markdown)
    {
        std::istringstream iss(markdown);
        std::string line;
        while (std::getline(iss, line))
        {
            if (Text::IsHeading1(line))
            {
                return line.substr(2); // Skip "# "
            }
        }
        return "Untitled Page";
    }
}