#include "parser.hpp"
#include "style.hpp"
#include "text.hpp"
#include "embed.hpp"
#include "snippet.hpp"
#include <sstream>
#include <cctype>

namespace Parser 
{
    std::string ProcessInline(const std::string &text)
    {
        std::string out = text;
        out = Embed::ProcessLinks(out);
        out = Snippet::ReplaceInlineCode(out);
        out = Text::ReplaceBoldItalic(out);
        out = Text::ReplaceBold(out);
        out = Text::ReplaceItalic(out);
        return out;
    }
    
    std::string ParseMarkdown(const std::string &markdown)
    {
        std::istringstream iss(markdown);
        std::string line;
        std::ostringstream html;
        bool inUl = false, inOl = false, inCodeBlock = false;
        bool inColumn = false, inCSS = false;
        std::ostringstream columnBuffer;
        std::vector<std::string> columnParts;
        std::ostringstream cssBuffer;

        while (std::getline(iss, line))
        {
            // Section markers:
            if (Style::IsSectionOpenLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << "<section>\n";
                continue;
            }
            if (Style::IsSectionCloseLine(line))
            {
                html << "</section>\n";
                continue;
            }
            // CSS markers:
            if (Style::IsCSSOpenLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                // Check if this is immediate CSS mode: if the line contains "]()".
                if (line.find("]()") != std::string::npos)
                {
                    std::string trimmed = Style::Trim(line);
                    size_t start = trimmed.find('[');
                    size_t end = trimmed.find(']', start + 1);
                    if (start != std::string::npos && end != std::string::npos && end > start)
                    {
                        std::string cssContent = trimmed.substr(start + 1, end - start - 1);
                        html << "<style>\n" << cssContent << "\n</style>\n";
                    }
                    // Immediately skip further lines until the close marker.
                    while (std::getline(iss, line))
                    {
                        if (Style::IsCSSCloseLine(line))
                            break;
                    }
                    continue;
                }
                else
                {
                    inCSS = true;
                    cssBuffer.str("");
                    cssBuffer.clear();
                    // Optionally extract any inline CSS from the marker
                    std::string trimmed = Style::Trim(line);
                    size_t start = trimmed.find('[');
                    size_t end = trimmed.find(']', start + 1);
                    if (start != std::string::npos && end != std::string::npos && end > start)
                    {
                        std::string initialCSS = trimmed.substr(start + 1, end - start - 1);
                        cssBuffer << initialCSS << "\n";
                    }
                    continue;
                }
            }
            if (Style::IsCSSCloseLine(line))
            {
                html << "<style>\n" << cssBuffer.str() << "\n</style>\n";
                inCSS = false;
                continue;
            }
            if (inCSS)
            {
                cssBuffer << line << "\n";
                continue;
            }
            
            // Column markers:
            if (Style::IsColumnOpenLine(line))
            {
                if (inColumn && !columnBuffer.str().empty())
                {
                    columnParts.push_back(columnBuffer.str());
                    columnBuffer.str("");
                    columnBuffer.clear();
                }
                else if (!inColumn)
                {
                    Text::CloseLists(html, inUl, inOl);
                    inColumn = true;
                    columnParts.clear();
                    columnBuffer.str("");
                    columnBuffer.clear();
                }
                continue;
            }
            if (Style::IsColumnCloseLine(line))
            {
                if (!columnBuffer.str().empty())
                {
                    columnParts.push_back(columnBuffer.str());
                }
                // Instead of using inline processing, run the full markdown parser on each column.
                for (auto &col : columnParts)
                {
                    col = Parser::ParseMarkdown(col);
                }
                html << Style::ApplyColumns(static_cast<int>(columnParts.size()), columnParts);
                inColumn = false;
                columnBuffer.str("");
                columnBuffer.clear();
                continue;
            }
            if (inColumn)
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
            
            // Existing markdown processing for other elements:
            if (Embed::IsCodeBlockLine(line))
            {
                if (!inCodeBlock)
                {
                    Text::CloseLists(html, inUl, inOl);
                    inCodeBlock = true;
                    html << Snippet::ParseCodeBlock(iss, line);
                    inCodeBlock = false;
                }
                continue;
            }
            if (line.empty())
            {
                Text::CloseLists(html, inUl, inOl);
                continue;
            }
            if (Embed::IsIFrameLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << Embed::ProcessIFrame(line);
                continue;
            }
            if (Embed::IsAudioLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << Embed::ProcessAudio(line);
                continue;
            }
            if (Embed::IsPictureLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << Embed::ProcessPictures(line);
                continue;
            }
            if (Embed::IsSvgLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << Embed::ProcessSvg(line);
                continue;
            }
            if (Text::IsHorizontalRuleLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << "<hr>\n";
                continue;
            }
            if (Text::IsTableLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << Text::ParseTable(iss, line);
                continue;
            }
            if (Embed::IsImageLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << Embed::ProcessImages(line);
                continue;
            }
            if (Embed::IsVideoLine(line))
            {
                Text::CloseLists(html, inUl, inOl);
                html << Embed::ProcessVideos(line);
                continue;
            }
            if (Text::IsHeading1(line))
            {
                Text::CloseLists(html, inUl, inOl);
                Text::AppendHeading(html, 1, line.substr(2));
            }
            else if (Text::IsHeading2(line))
            {
                Text::CloseLists(html, inUl, inOl);
                Text::AppendHeading(html, 2, line.substr(3));
            }
            else if (Text::IsHeading3(line))
            {
                Text::CloseLists(html, inUl, inOl);
                Text::AppendHeading(html, 3, line.substr(4));
            }
            else if (Text::IsUnorderedListLine(line))
            {
                if (!inUl)
                {
                    Text::CloseLists(html, inUl, inOl);
                    html << "<ul>\n";
                    inUl = true;
                }
                Text::AppendListItem(html, line.substr(2));
            }
            else if (Text::IsNumberedListLine(line))
            {
                if (!inOl)
                {
                    Text::CloseLists(html, inUl, inOl);
                    html << "<ol>\n";
                    inOl = true;
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
                Text::CloseLists(html, inUl, inOl);
                Text::AppendParagraph(html, line);
            }
        }
        Text::CloseLists(html, inUl, inOl);
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
}