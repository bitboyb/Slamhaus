#include "section.hpp"
#include "attributes.hpp"
#include "text.hpp"
#include "../util/string.hpp"

namespace Section
{
    bool HandleHeroOpen(const std::string &line,
                        std::ostringstream &html,
                        Parser::ParseState &pState)
    {
        std::string trimmed = String::Trim(line);
        if (trimmed.rfind(":hero[", 0) != 0)
        {
            return false;
        }

        Text::CloseLists(html, pState);

        auto attrs = Attributes::ParseAttributes(line, ":hero");
        std::string type = "standard";
        std::string media;
        std::string id;

        if (attrs.count("type")) 
        {
            type = attrs["type"];
        }
        if (attrs.count("media")) 
        {
            media = attrs["media"];
        }
        if (attrs.count("id"))
        {
            id = attrs["id"];
        } 

        if (type == "video") 
        {
            html << "<section class=\"hero-video\"";
            if (!id.empty())
            {
                html << " id=\"" << id << "\"";
            }
            html << ">\n";
            html << "<video src=\"" << media
                << "\" autoplay muted loop playsinline preload=\"auto\"></video>\n";
            html << "<div class=\"hero-content\">\n";
        }
        else if (type == "image") 
        {
            html << "<section class=\"hero-image\"";
            if (!id.empty()) 
            {
                html << " id=\"" << id << "\"";
            }
            html << " style=\"background-image: url('" << media << "');\">\n";
            html << "<div class=\"hero-content\">\n";
        }
        else 
        {
            html << "<section class=\"hero\"";
            if (!id.empty()) 
            {
                html << " id=\"" << id << "\"";
            }
            html << ">\n";
        }
        return true;
    }

    bool HandleHeroClose(const std::string &line,
                        std::ostringstream &html)
    {
        if (String::Trim(line) != ":/hero")
            return false;

        html << "  </div>\n";
        html << "</section>\n";
        return true;
    }

    bool HandleSectionOpen(const std::string &line,
                           std::ostringstream &html,
                           Parser::ParseState &pState)
    {
        std::string trimmed = String::Trim(line);
        if (trimmed.rfind(":section[", 0) != 0) 
        {
            return false;
        }

        Text::CloseLists(html, pState);
        html << "<section";

        auto attrs = Attributes::ParseAttributes(line, ":section");
        if (attrs.count("id")) 
        {
            html << " id=\"" << attrs["id"] << "\"";
        }
        html << ">\n";
        return true;
    }

    bool HandleSectionClose(const std::string &line,
                            std::ostringstream &html)
    {
        if (String::Trim(line) != ":/section") 
        {
            return false;
        }

        html << "</section>\n";
        return true;
    }

    bool HandleColumnOpen(const std::string &line,
                          std::ostringstream &html,
                          Parser::ParseState &pState,
                          Parser::ColumnState &cState)
    {
        std::string trimmed = String::Trim(line);
        if (trimmed.rfind(":column[", 0) != 0) 
        {
            return false;
        }

        if (pState.inColumn && !cState.columnBuffer.str().empty()) 
        {
            cState.columnParts.push_back(cState.columnBuffer.str());
            cState.columnOpenLines.push_back(line);
            cState.columnBuffer.str("");
            cState.columnBuffer.clear();
        }
        else if (!pState.inColumn) 
        {
            Text::CloseLists(html, pState);
            pState.inColumn = true;
            cState.columnParts.clear();
            cState.columnOpenLines.clear();
            cState.columnBuffer.str("");
            cState.columnBuffer.clear();
            cState.columnOpenLines.push_back(line);
        }

        return true;
    }

    std::string ApplyColumns(int fallbackNumber,
                             const std::vector<std::string>& text,
                             const std::vector<std::string>& openLines)
    {
        std::ostringstream oss;
        int numColumns = fallbackNumber;

        if (!openLines.empty())
        {
            auto attrs = Attributes::ParseAttributes(openLines[0], ":column");
            if (attrs.count("size"))
            {
                try
                {
                    numColumns = std::stoi(attrs["size"]);
                }
                catch (...) {}
            }
        }

        oss << "<div class=\"columns-container responsive-columns\">";
        for (int i = 0; i < numColumns; i++)
        {
            std::string content = i < (int)text.size() ? text[i] : "";
            std::string openLine = i < (int)openLines.size() ? openLines[i] : "";

            oss << "<div class=\"column\"";
            auto attrs = Attributes::ParseAttributes(openLine, ":column");
            if (attrs.count("id"))
            {
                oss << " id=\"" << attrs["id"] << "\"";
            }
            oss << ">";
            oss << content << "</div>";
        }
        oss << "</div>";
        return oss.str();
    }


    bool HandleColumnClose(const std::string &line,
                           std::ostringstream &html,
                           Parser::ParseState &pState,
                           Parser::ColumnState &cState,
                           SEO::SEOData *seo)
    {
        if (String::Trim(line) != ":/column") 
        {
            return false;
        }

        if (!cState.columnBuffer.str().empty()) 
        {
            cState.columnParts.push_back(cState.columnBuffer.str());
        }
        for (auto & col : cState.columnParts) 
        {
            col = Parser::ParseMarkdown(col, seo);
        }
        html << ApplyColumns(
            static_cast<int>(cState.columnParts.size()),
            cState.columnParts,
            cState.columnOpenLines
        );
        pState.inColumn = false;
        cState.columnBuffer.str("");
        cState.columnBuffer.clear();
        return true;
    }

    bool HandleDivOpen(const std::string &line,
                        std::ostringstream &html,
                        Parser::ParseState &pState)
    {
        std::string trimmed = String::Trim(line);
        if (trimmed.rfind(":div[", 0) != 0) 
        {
            return false;
        }

        Text::CloseLists(html, pState);
        html << "<div";

        auto attrs = Attributes::ParseAttributes(trimmed, ":div");
        if (attrs.count("id")) 
        {
            html << " id=\"" << attrs["id"] << "\"";
        }
        html << ">\n";
        return true;
    }

    bool HandleDivClose(const std::string &line,
                        std::ostringstream &html)
    {
        if (String::Trim(line) != ":/div") 
        {
            return false;
        }
        html << "</div>\n";
        return true;
    }

    bool HandleCanvasOpen(const std::string &line,
                        std::ostringstream &html,
                        Parser::ParseState &pState)
    {
        std::string trimmed = String::Trim(line);
        if (trimmed.rfind(":canvas[", 0) != 0)
        {
            return false;
        }

        Text::CloseLists(html, pState);

        auto attrs = Attributes::ParseAttributes(trimmed, ":canvas");

        std::string id = attrs.count("id") ? attrs["id"] : "canvas";
        std::string width = attrs.count("width") ? attrs["width"] : "64";
        std::string height = attrs.count("height") ? attrs["height"] : "64";

        html << "<div class=\"canvas-wrapper\">\n";

        html << "<div class=\"canvas\" id=\"" << id
            << "\" data-width=\"" << width
            << "\" data-height=\"" << height
            << "\">\n";

        html << "<script type=\"module\">\n";
        html << "function fitCanvasSize_" << id << "() {\n";
        html << "  const canvas = document.getElementById('" << id << "');\n";
        html << "  const charsX = parseInt(canvas.dataset.width);\n";
        html << "  const charsY = parseInt(canvas.dataset.height);\n";
        html << "  canvas.style.width = `${charsX}ch`;\n";
        html << "  canvas.style.height = `${charsY}em`;\n";
        html << "}\n";
        html << "window.addEventListener('load', fitCanvasSize_" << id << ");\n";
        html << "window.addEventListener('resize', fitCanvasSize_" << id << ");\n";
        html << "</script>\n";

        return true;
    }


    bool HandleCanvasClose(const std::string &line,
                        std::ostringstream &html)
    {
        if (String::Trim(line) != ":/canvas") {
            return false;
        }
        html << "</div>\n";
        html << "</div>\n";
        return true;
    }

    bool HandleBlockElements(const std::string &line,
                             std::ostringstream &html,
                             Parser::ParseState &pState,
                             Parser::ColumnState &cState,
                             SEO::SEOData *seo)
    {
        if (HandleHeroOpen(line, html, pState)) 
        {
            return true;
        }
        if (HandleHeroClose(line, html)) 
        {
            return true;
        }
        if (HandleSectionOpen(line, html, pState))
        {
            return true;
        }
        if (HandleDivOpen(line, html, pState))
        {
            return true;
        }
        if (HandleCanvasOpen(line, html, pState))
        {
            return true;
        }
        if (HandleSectionClose(line, html)) 
        {
            return true;
        }
        if (HandleColumnOpen(line, html, pState, cState)) 
        {
            return true;
        }
        if (HandleColumnClose(line, html, pState, cState, seo)) 
        {
            return true;
        }
        if (HandleDivClose(line, html))
        {
            return true;
        }
        if (HandleCanvasClose(line, html))
        {
            return true;
        }
        if (pState.inColumn)
        {
            if (line == "---")
            {
                cState.columnParts.push_back(cState.columnBuffer.str());
                cState.columnBuffer.str("");
                cState.columnBuffer.clear();
            }
            else
            {
                cState.columnBuffer << line << "\n";
            }
            return true;
        }
        return false;
    }
}
