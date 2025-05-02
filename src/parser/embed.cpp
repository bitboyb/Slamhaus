#include "embed.hpp"
#include "attributes.hpp"
#include "text.hpp"
#include "parser.hpp"

#include <sstream>

namespace Embed
{

    static void EmitAttrs(std::ostringstream &oss,
                          const std::map<std::string,std::string> &attrs)
    {
        // all our attrs (id, class, style, width, height, etc.) go straight onto the tag
        for (auto const &kv : attrs)
        {
            oss << " " << kv.first;
            if (!kv.second.empty())
            {
                oss << "=\"" << kv.second << "\"";
            }
        }
    }

    bool IsIFrameLine(const std::string &line)
    {
        return line.rfind("!iframe[", 0) == 0 && line.back() == ']';
    }

    bool IsImageLine(const std::string &line)
    {
        return line.rfind("![", 0) == 0 || line.rfind("!image[", 0) == 0;
    }

    bool IsVideoLine(const std::string &line)
    {
        return line.rfind("!video[", 0) == 0 || line.rfind("!videos[", 0) == 0;
    }

    bool IsAudioLine(const std::string &line)
    {
        return line.rfind("!audio[", 0) == 0;
    }

    bool IsPictureLine(const std::string &line)
    {
        return line.rfind("!picture[", 0) == 0 || line.rfind("!pictures[", 0) == 0;
    }

    bool IsSvgLine(const std::string &line)
    {
        return line.rfind("!svg[", 0) == 0;
    }

    // height: and width: tokens still map into style
    void ExtractContentAndStyle(const std::string &raw,
                                std::string &content,
                                std::string &style)
    {
        std::istringstream iss(raw);
        content.clear();
        style.clear();
        std::string token;
        while (iss >> token)
        {
            auto colon = token.find(':');
            if (colon != std::string::npos)
            {
                auto key   = token.substr(0, colon);
                auto value = token.substr(colon+1);
                if (key == "height" || key == "width")
                {
                    style += key + ":" + (value == "max" ? "100%" : value) + "; ";
                    continue;
                }
            }
            if (!content.empty())
            {
                content += " ";
            }
            content += token;
        }
        if (content.empty())
        {
            content = raw;
        }
    }

    std::string ProcessImages(const std::string &line)
    {
        // find bracketed body and parenthesized URL
        auto open  = line.find('[');
        auto close = line.find(']', open+1);
        auto paren = line.find('(', close);
        auto end   = line.find(')', paren+1);

        std::string body = line.substr(open+1, close-open-1);
        std::string url  = (paren!=std::string::npos && end!=std::string::npos)
                           ? line.substr(paren+1, end-paren-1)
                           : "";

        // parse everything inside [ ]
        auto attrs = Attributes::ParseAttributes(line, "![");
        // if you want to pull plain alt text out too
        std::string alt;
        std::string styleFromTokens;
        ExtractContentAndStyle(body, alt, styleFromTokens);

        if (!styleFromTokens.empty())
        {
            attrs["style"] += styleFromTokens;
        }
        if (!alt.empty() && !attrs.count("alt"))
        {
            attrs["alt"] = alt;
        }

        // wrap in link if provided
        std::string link;
        if (attrs.count("link"))
        {
            link = attrs["link"];
            attrs.erase("link");
        }
        else if (attrs.count("href"))
        {
            link = attrs["href"];
            attrs.erase("href");
        }

        std::ostringstream oss;
        oss << "<img src=\"" << url << "\"";
        EmitAttrs(oss, attrs);
        oss << ">";

        if (!link.empty())
        {
            return "<a href=\"" + link + "\">" + oss.str() + "</a>";
        }
        return oss.str();
    }

    std::string ProcessIFrame(const std::string &line)
    {
        auto open  = line.find('[');
        auto close = line.find(']', open+1);
        std::string body = line.substr(open+1, close-open-1);

        std::string url, styleFromTokens;
        ExtractContentAndStyle(body, url, styleFromTokens);

        auto attrs = Attributes::ParseAttributes(line, "!iframe");
        if (!styleFromTokens.empty())
        {
            attrs["style"] += styleFromTokens;
        }

        std::ostringstream oss;
        oss << "<div class=\"embed-container\">\n"
            << "<iframe src=\"" << url << "\"";
        EmitAttrs(oss, attrs);
        oss << " frameborder=\"0\" allowfullscreen "
               "referrerpolicy=\"strict-origin-when-cross-origin\"></iframe>\n"
               "</div>\n";
        return oss.str();
    }

    std::string ProcessVideos(const std::string &line)
    {
        std::string prefix = (line.rfind("!videos[",0)==0 ? "!videos[" : "!video[");
        auto open  = line.find('[', prefix.size());
        auto close = line.find(']', open+1);
        auto paren = line.find('(', close);
        auto end   = line.find(')', paren+1);

        std::string body = line.substr(open+1, close-open-1);
        std::string url  = (paren!=std::string::npos && end!=std::string::npos)
                           ? line.substr(paren+1,end-paren-1)
                           : "";

        std::string desc, styleFromTokens;
        ExtractContentAndStyle(body, desc, styleFromTokens);

        auto attrs = Attributes::ParseAttributes(line, prefix);
        if (!styleFromTokens.empty())
        {
            attrs["style"] += styleFromTokens;
        }
        if (!desc.empty() && !attrs.count("title"))
        {
            attrs["title"] = desc;
        }

        std::ostringstream oss;
        oss << "<video src=\"" << url << "\" controls";
        EmitAttrs(oss, attrs);
        oss << ">Your browser does not support the video tag.</video>";
        return oss.str();
    }

    std::string ProcessAudio(const std::string &line)
    {
        auto open  = line.find('[');
        auto close = line.find(']', open+1);
        auto paren = line.find('(', close);
        auto end   = line.find(')', paren+1);

        std::string body = line.substr(open+1, close-open-1);
        std::string url  = (paren!=std::string::npos && end!=std::string::npos)
                           ? line.substr(paren+1,end-paren-1)
                           : "";

        std::string desc, styleFromTokens;
        ExtractContentAndStyle(body, desc, styleFromTokens);

        auto attrs = Attributes::ParseAttributes(line, "!audio");
        if (!styleFromTokens.empty())
        {
            attrs["style"] += styleFromTokens;
        }
        if (!desc.empty() && !attrs.count("title"))
        {
            attrs["title"] = desc;
        }

        std::ostringstream oss;
        oss << "<audio src=\"" << url << "\" controls";
        EmitAttrs(oss, attrs);
        oss << ">Your browser does not support the audio element.</audio>";
        return oss.str();
    }

    std::string ProcessPictures(const std::string &line)
    {
        // same as ProcessImages but wrapped in <picture>…</picture>
        std::string img = ProcessImages(line);
        return "<picture>" + img + "</picture>";
    }

    std::string ProcessSvg(const std::string &line)
    {
        auto open  = line.find('[');
        auto close = line.find(']', open+1);
        auto paren = line.find('(', close);
        auto end   = line.find(')', paren+1);

        std::string body = line.substr(open+1, close-open-1);
        std::string url  = (paren!=std::string::npos && end!=std::string::npos)
                           ? line.substr(paren+1,end-paren-1)
                           : "";

        std::string desc, styleFromTokens;
        ExtractContentAndStyle(body, desc, styleFromTokens);

        auto attrs = Attributes::ParseAttributes(line, "!svg");
        if (!styleFromTokens.empty())
        {
            attrs["style"] += styleFromTokens;
        }
        if (!desc.empty() && !attrs.count("title"))
        {
            attrs["title"] = desc;
        }

        std::ostringstream oss;
        oss << "<object data=\"" << url << "\" type=\"image/svg+xml\"";
        EmitAttrs(oss, attrs);
        oss << ">Your browser does not support SVG.</object>";
        return oss.str();
    }

    bool HandleEmbeds(const std::string &line,
                      std::ostringstream    &html,
                      Parser::ParseState    &pState)
    {
        Text::CloseLists(html, pState);

        if (IsIFrameLine(line))  { html << ProcessIFrame(line);    return true; }
        if (IsAudioLine(line))   { html << ProcessAudio(line);     return true; }
        if (IsPictureLine(line)) { html << ProcessPictures(line);  return true; }
        if (IsSvgLine(line))     { html << ProcessSvg(line);       return true; }
        if (IsImageLine(line))   { html << ProcessImages(line);    return true; }
        if (IsVideoLine(line))   { html << ProcessVideos(line);    return true; }
        return false;
    }

}
