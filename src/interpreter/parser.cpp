#include "parser.hpp"
#include "interaction.hpp"
#include "form.hpp"
#include "seo.hpp"
#include "section.hpp"
#include "text.hpp"
#include "embed.hpp"
#include "snippet.hpp"
#include <sstream>
#include <cctype>

namespace Parser 
{
    std::string ParseMarkdown(const std::string& markdown, 
                              SEO::SEOData* seo)
    {
        std::istringstream iss(markdown);
        std::ostringstream html;
        std::string line;
    
        Parser::ParseState pState;
        Parser::ColumnState cState;
    
        while (std::getline(iss, line))
        {
            if (SEO::HandleMetaLines(line, seo)) continue;
            if (Section::HandleBlockElements(line, html, pState, cState, seo)) continue;
            if (Form::HandleFormElements(line, html, pState)) continue;
            if (Interaction::IsInteraction(line, html, pState)) continue;
            if (Embed::HandleEmbeds(line, html, pState)) continue;
            if (Snippet::HandleCodeOrTable(line, iss, html, pState)) continue;
            if (Text::HandleTextBlocks(line, html, pState)) continue;
        }
        Text::CloseLists(html, pState);
        return html.str();
    }
}