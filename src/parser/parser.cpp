#include "parser.hpp"
#include "interaction.hpp"
#include "form.hpp"
#include "seo.hpp"
#include "section.hpp"
#include "text.hpp"
#include "embed.hpp"
#include "snippet.hpp"
#include <sstream>
#include "../wasm/script.hpp"
#include <cctype>

namespace Parser 
{
    std::string ParseMarkdown(const std::string& markdown, 
                              SEO::SEOData* seo)
    {
        std::istringstream iss(markdown);
        std::ostringstream html;
        std::string line;
        std::string buffer;
    
        Parser::ParseState pState;
        Parser::ColumnState cState;
    
        while (std::getline(iss, line))
        {
            std::string trimmed = line;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));
            trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
            std::string raw;
            if (!pState.isBuffering) 
            {
                if (!trimmed.empty() &&
                    (trimmed[0]==':' || trimmed[0]=='@' ||
                    trimmed[0]=='?' || trimmed[0]=='$') &&
                    trimmed.find('[') != std::string::npos &&
                    trimmed.find("](") == std::string::npos)
                {
                    pState.isBuffering = true;
                    buffer = line;
                    continue;
                }
                else 
                {
                    raw = line;
                }
            }
            else 
            {
                buffer += "\n" + line;
                if (buffer.find("](") != std::string::npos) 
                {
                    pState.isBuffering = false;
                    raw = buffer;
                } 
                else 
                {
                    continue;
                }
            }
            if (!raw.empty() && raw[0]=='<') 
            {
                Text::CloseLists(html, pState);
                html << raw << "\n";
                continue;
            }
            if (Script::HandleScript(raw, iss, html, pState)) continue;
            if (SEO::HandleMetaLines(raw, seo)) continue;
            if (Section::HandleBlockElements(raw, html, pState, cState, seo)) continue;
            if (Form::HandleFormElements(raw, html, pState)) continue;
            if (Interaction::IsInteraction(raw, html, pState)) continue;
            if (Embed::HandleEmbeds(raw, html, pState)) continue;
            if (Snippet::HandleCodeOrTable(raw, iss, html, pState)) continue;
            if (Text::HandleTextBlocks(raw, html, pState)) continue;
        }
        Text::CloseLists(html, pState);
        return html.str();
    }
}