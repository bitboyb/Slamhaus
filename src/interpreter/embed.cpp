#include <sstream>
#include <cctype>
#include "parser.hpp"
#include "text.hpp"

namespace Embed
{



    bool IsIFrameLine(const std::string &line) 
    {
        return line.rfind("!iframe[", 0) == 0 && !line.empty() && line.back() == ']';
    }

    bool IsImageLine(const std::string &line) 
    {
        return line.rfind("![", 0) == 0 || line.rfind("!image[", 0) == 0;
    }
    
    bool IsVideoLine(const std::string &line)
    {
        return line.rfind("!videos[", 0) == 0 || line.rfind("!video[", 0) == 0;
    }

    bool IsAudioLine(const std::string &line)
    {
        return line.rfind("!audio[", 0) == 0;
    }
    
    bool IsPictureLine(const std::string &line)
    {
        return line.rfind("!pictures[", 0) == 0 || line.rfind("!picture[", 0) == 0;
    }
    
    bool IsSvgLine(const std::string &line)
    {
        return line.rfind("!svg[", 0) == 0;
    }

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
            size_t colonPos = token.find(':');
            if (colonPos != std::string::npos) 
            {
                std::string key = token.substr(0, colonPos);
                std::string value = token.substr(colonPos + 1);
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

    std::string ProcessImages(const std::string &input) 
    {
        std::string out = input;
        size_t pos = 0;
        while ((pos = out.find("![", pos)) != std::string::npos) 
        {
            size_t endAlt = out.find("]", pos + 2);
            if ((endAlt != std::string::npos) && ((endAlt + 1) < out.size()) && (out[endAlt + 1] == '(')) 
            {
                size_t endUrl = out.find(")", endAlt + 2);
                if (endUrl != std::string::npos) 
                {
                    std::string rawAlt = out.substr(pos + 2, endAlt - (pos + 2));
                    std::string url = out.substr(endAlt + 2, endUrl - (endAlt + 2));
                    std::string description, styleAttr;
                    ExtractContentAndStyle(rawAlt, description, styleAttr);
                    std::string imgTag = "<img src=\"" + url + "\" alt=\"" + description + "\"";
                    if (!styleAttr.empty()) 
                    {
                        imgTag += " style=\"" + styleAttr + "\"";
                    }
                    imgTag += ">";
                    out.replace(pos, endUrl - pos + 1, imgTag);
                    pos += imgTag.size();
                } 
                else 
                {
                    pos = endAlt + 1;
                }
            } 
            else 
            {
                pos += 2;
            }
        }
        return out;
    }

    std::string ProcessIFrame(const std::string &input) 
    {
        std::string out = input;
        size_t pos = out.find("!iframe[");
        if (pos != std::string::npos && out.back() == ']') 
        {
            size_t start = out.find('[', pos);
            size_t end = out.find(']', pos);
            if (start != std::string::npos && end != std::string::npos && end > start) 
            {
                std::string rawContent = out.substr(start + 1, end - start - 1);
                std::string url, styleAttr;
                ExtractContentAndStyle(rawContent, url, styleAttr);
                std::string iframeTag = "<div class=\"embed-container\">\n"
                                        "<iframe src=\"" + url + "\"";
                if (!styleAttr.empty()) 
                {
                    iframeTag += " style=\"" + styleAttr + "\"";
                }
                iframeTag += " frameborder=\"0\" allowfullscreen "
                             "referrerpolicy=\"strict-origin-when-cross-origin\"></iframe>\n"
                             "</div>\n";
                out.replace(pos, end - pos + 1, iframeTag);
            }
        }
        return out;
    }

    std::string ProcessVideos(const std::string &input) 
    {
        std::string out = input;
        size_t pos = out.find("!videos[");
        std::string prefix;
        if (pos == std::string::npos) {
            pos = out.find("!video[");
            prefix = "!video[";
        } else {
            prefix = "!videos[";
        }
        
        if (pos != std::string::npos)
        {
            size_t prefixLen = prefix.size();
            size_t endAlt = out.find("]", pos + prefixLen);
            if (endAlt != std::string::npos && (endAlt + 1) < out.size() && out[endAlt + 1] == '(')
            {
                size_t endUrl = out.find(")", endAlt + 2);
                if (endUrl != std::string::npos)
                {
                    std::string rawAlt = out.substr(pos + prefixLen, endAlt - (pos + prefixLen));
                    std::string url = out.substr(endAlt + 2, endUrl - (endAlt + 2));
                    std::string description, styleAttr;
                    ExtractContentAndStyle(rawAlt, description, styleAttr);
                    std::string videoTag = "<video src=\"" + url + "\" controls";
                    if (!styleAttr.empty())
                    {
                        videoTag += " style=\"" + styleAttr + "\"";
                    }
                    videoTag += ">";
                    videoTag += "Your browser does not support the video tag.";
                    videoTag += "</video>";
                    out.replace(pos, endUrl - pos + 1, videoTag);
                }
            }
        }
        return out;
    }

    std::string ProcessAudio(const std::string &input) 
    {
        std::string out = input;
        size_t pos = out.find("!audio[");
        std::string prefix = "!audio[";
        if (pos != std::string::npos)
        {
            size_t prefixLen = prefix.size();
            size_t endAlt = out.find("]", pos + prefixLen);
            if (endAlt != std::string::npos && (endAlt + 1) < out.size() && out[endAlt + 1] == '(')
            {
                size_t endUrl = out.find(")", endAlt + 2);
                if (endUrl != std::string::npos)
                {
                    std::string rawAlt = out.substr(pos + prefixLen, endAlt - (pos + prefixLen));
                    std::string url = out.substr(endAlt + 2, endUrl - (endAlt + 2));
                    std::string description, styleAttr;
                    ExtractContentAndStyle(rawAlt, description, styleAttr);
                    std::string audioTag = "<audio src=\"" + url + "\" controls";
                    if (!styleAttr.empty())
                    {
                        audioTag += " style=\"" + styleAttr + "\"";
                    }
                    audioTag += ">";
                    audioTag += "Your browser does not support the audio element.";
                    audioTag += "</audio>";
                    out.replace(pos, endUrl - pos + 1, audioTag);
                }
            }
        }
        return out;
    }
    
    std::string ProcessPictures(const std::string &input) 
    {
        std::string out = input;
        size_t pos = out.find("!pictures[");
        std::string prefix;
        if (pos == std::string::npos) {
            pos = out.find("!picture[");
            prefix = "!picture[";
        } else {
            prefix = "!pictures[";
        }
        if (pos != std::string::npos)
        {
            size_t prefixLen = prefix.size();
            size_t endAlt = out.find("]", pos + prefixLen);
            if (endAlt != std::string::npos && (endAlt + 1) < out.size() && out[endAlt + 1] == '(')
            {
                size_t endUrl = out.find(")", endAlt + 2);
                if (endUrl != std::string::npos)
                {
                    std::string rawAlt = out.substr(pos + prefixLen, endAlt - (pos + prefixLen));
                    std::string url = out.substr(endAlt + 2, endUrl - (endAlt + 2));
                    std::string description, styleAttr;
                    ExtractContentAndStyle(rawAlt, description, styleAttr);
                    std::string pictureTag = "<picture><img src=\"" + url + "\" alt=\"" + description + "\"";
                    if (!styleAttr.empty()) {
                        pictureTag += " style=\"" + styleAttr + "\"";
                    }
                    pictureTag += "></picture>";
                    out.replace(pos, endUrl - pos + 1, pictureTag);
                }
            }
        }
        return out;
    }
    
    std::string ProcessSvg(const std::string &input) 
    {
        std::string out = input;
        size_t pos = out.find("!svg[");
        if (pos != std::string::npos)
        {
            size_t prefixLen = 5;
            size_t endAlt = out.find("]", pos + prefixLen);
            if (endAlt != std::string::npos && (endAlt + 1) < out.size() && out[endAlt + 1] == '(')
            {
                size_t endUrl = out.find(")", endAlt + 2);
                if (endUrl != std::string::npos)
                {
                    std::string rawAlt = out.substr(pos + prefixLen, endAlt - (pos + prefixLen));
                    std::string url = out.substr(endAlt + 2, endUrl - (endAlt + 2));
                    std::string description, styleAttr;
                    ExtractContentAndStyle(rawAlt, description, styleAttr);
                    std::string svgTag = "<object data=\"" + url + "\" type=\"image/svg+xml\"";
                    if (!styleAttr.empty())
                    {
                        svgTag += " style=\"" + styleAttr + "\"";
                    }
                    svgTag += ">";
                    svgTag += "Your browser does not support SVG.";
                    svgTag += "</object>";
                    out.replace(pos, endUrl - pos + 1, svgTag);
                }
            }
        }
        return out;
    }

    bool HandleEmbeds(const std::string& line, 
                      std::ostringstream& html, 
                      Parser::ParseState& pState)
    {
        if (IsIFrameLine(line)) 
        {
            Text::CloseLists(html, pState);
            html << Embed::ProcessIFrame(line);
            return true;
        }
        if (IsAudioLine(line)) 
        {
            Text::CloseLists(html, pState);
            html << Embed::ProcessAudio(line);
            return true;
        }
        if (IsPictureLine(line)) 
        {
            Text::CloseLists(html, pState);
            html << Embed::ProcessPictures(line);
            return true;
        }
        if (IsSvgLine(line)) 
        {
            Text::CloseLists(html, pState);
            html << Embed::ProcessSvg(line);
            return true;
        }
        if (Embed::IsImageLine(line)) 
        {
            Text::CloseLists(html, pState);
            html << Embed::ProcessImages(line);
            return true;
        }
        if (IsVideoLine(line)) 
        {
            Text::CloseLists(html, pState);
            html << Embed::ProcessVideos(line);
            return true;
        }
        return false;
    }
}