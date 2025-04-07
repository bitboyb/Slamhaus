#ifndef EMBED_HPP
#define EMBED_HPP

#include <string>

namespace Embed
{
    bool IsCodeBlockLine(const std::string &line);
    bool IsIFrameLine(const std::string &line);
    bool IsImageLine(const std::string &line);
    bool IsVideoLine(const std::string &line);
    bool IsAudioLine(const std::string &line);
    bool IsPictureLine(const std::string &line);
    bool IsSvgLine(const std::string &line);

    std::string ProcessImages(const std::string &input);
    std::string ProcessLinks(const std::string &input);
    std::string ProcessIFrame(const std::string &input);
    std::string ProcessVideos(const std::string &input);
    std::string ProcessAudio(const std::string &input);
    std::string ProcessPictures(const std::string &input);
    std::string ProcessSvg(const std::string &input);
}

#endif // EMBED_HPP
