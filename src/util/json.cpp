#include "json.hpp"
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace Util::JSON {

namespace {

struct Token {
  enum Type {
    T_LCURLY,
    T_RCURLY,
    T_LBRACKET,
    T_RBRACKET,
    T_COLON,
    T_COMMA,
    T_STRING,
    T_NUMBER,
    T_TRUE,
    T_FALSE,
    T_NULL,
    T_EOF,
    T_ERROR
  };
  Type type;
  std::string value;
};

struct LexerState {
  const std::string &input;
  size_t pos;
};

void SkipWhitespace(LexerState &state) {
  while (state.pos < state.input.size() && isspace(state.input[state.pos]))
    state.pos++;
}

Token Advance(LexerState &state, Token::Type type) {
  state.pos++;
  return {type, ""};
}

Token ReadString(LexerState &state) {
  state.pos++; // Skip opening quote
  std::string val;
  while (state.pos < state.input.size()) {
    char c = state.input[state.pos];
    if (c == '"') {
      state.pos++;
      return {Token::T_STRING, val};
    }
    if (c == '\\') {
      state.pos++;
      if (state.pos < state.input.size()) {
        char esc = state.input[state.pos];
        if (esc == '"')
          val += '"';
        else if (esc == '\\')
          val += '\\';
        else if (esc == '/')
          val += '/';
        else if (esc == 'b')
          val += '\b';
        else if (esc == 'f')
          val += '\f';
        else if (esc == 'n')
          val += '\n';
        else if (esc == 'r')
          val += '\r';
        else if (esc == 't')
          val += '\t';
        else
          val += esc;
      }
    } else {
      val += c;
    }
    state.pos++;
  }
  return {Token::T_ERROR, ""};
}

Token ReadNumber(LexerState &state) {
  size_t start = state.pos;
  if (state.input[state.pos] == '-')
    state.pos++;
  while (state.pos < state.input.size() && isdigit(state.input[state.pos]))
    state.pos++;
  if (state.pos < state.input.size() && state.input[state.pos] == '.') {
    state.pos++;
    while (state.pos < state.input.size() && isdigit(state.input[state.pos]))
      state.pos++;
  }
  return {Token::T_NUMBER, state.input.substr(start, state.pos - start)};
}

Token ReadKeyword(LexerState &state) {
  size_t start = state.pos;
  while (state.pos < state.input.size() && isalpha(state.input[state.pos]))
    state.pos++;
  std::string word = state.input.substr(start, state.pos - start);
  if (word == "true")
    return {Token::T_TRUE, ""};
  if (word == "false")
    return {Token::T_FALSE, ""};
  if (word == "null")
    return {Token::T_NULL, ""};
  return {Token::T_ERROR, ""};
}

Token NextToken(LexerState &state) {
  SkipWhitespace(state);
  if (state.pos >= state.input.size())
    return {Token::T_EOF, ""};

  char c = state.input[state.pos];
  if (c == '{')
    return Advance(state, Token::T_LCURLY);
  if (c == '}')
    return Advance(state, Token::T_RCURLY);
  if (c == '[')
    return Advance(state, Token::T_LBRACKET);
  if (c == ']')
    return Advance(state, Token::T_RBRACKET);
  if (c == ':')
    return Advance(state, Token::T_COLON);
  if (c == ',')
    return Advance(state, Token::T_COMMA);
  if (c == '"')
    return ReadString(state);
  if (isdigit(c) || c == '-')
    return ReadNumber(state);
  if (isalpha(c))
    return ReadKeyword(state);

  return Advance(state, Token::T_ERROR);
}

Node ParseValue(LexerState &state, Token &tok) {
  if (tok.type == Token::T_NULL) {
    tok = NextToken(state);
    return Node{Null{}};
  }
  if (tok.type == Token::T_TRUE) {
    tok = NextToken(state);
    return Node{true};
  }
  if (tok.type == Token::T_FALSE) {
    tok = NextToken(state);
    return Node{false};
  }
  if (tok.type == Token::T_NUMBER) {
    double val = std::atof(tok.value.c_str());
    tok = NextToken(state);
    return Node{val};
  }
  if (tok.type == Token::T_STRING) {
    std::string val = tok.value;
    tok = NextToken(state);
    return Node{val};
  }
  if (tok.type == Token::T_LBRACKET) {
    tok = NextToken(state);
    Array arr;
    if (tok.type != Token::T_RBRACKET) {
      while (true) {
        arr.push_back(ParseValue(state, tok));
        if (tok.type == Token::T_RBRACKET)
          break;
        if (tok.type != Token::T_COMMA)
          return Node{Null{}}; // Error
        tok = NextToken(state);
      }
    }
    tok = NextToken(state); // Skip ]
    return Node{arr};
  }
  if (tok.type == Token::T_LCURLY) {
    tok = NextToken(state);
    Object obj;
    if (tok.type != Token::T_RCURLY) {
      while (true) {
        if (tok.type != Token::T_STRING)
          return Node{Null{}}; // Error
        std::string key = tok.value;
        tok = NextToken(state);
        if (tok.type != Token::T_COLON)
          return Node{Null{}}; // Error
        tok = NextToken(state);
        obj[key] = ParseValue(state, tok);
        if (tok.type == Token::T_RCURLY)
          break;
        if (tok.type != Token::T_COMMA)
          return Node{Null{}}; // Error
        tok = NextToken(state);
      }
    }
    tok = NextToken(state); // Skip }
    return Node{obj};
  }
  return Node{Null{}};
}

void StringifyValue(const Node &node, std::stringstream &ss, bool pretty,
                    int depth) {
  if (IsNull(node)) {
    ss << "null";
  } else if (IsBool(node)) {
    ss << (AsBool(node) ? "true" : "false");
  } else if (IsNumber(node)) {
    ss << AsNumber(node);
  } else if (IsString(node)) {
    ss << "\"" << AsString(node) << "\"";
  } else if (IsArray(node)) {
    ss << "[";
    if (pretty)
      ss << "\n";
    const auto &arr = AsArray(node);
    for (size_t i = 0; i < arr.size(); ++i) {
      if (pretty)
        ss << std::string((depth + 1) * 2, ' ');
      StringifyValue(arr[i], ss, pretty, depth + 1);
      if (i < arr.size() - 1)
        ss << ",";
      if (pretty)
        ss << "\n";
    }
    if (pretty)
      ss << std::string(depth * 2, ' ');
    ss << "]";
  } else if (IsObject(node)) {
    ss << "{";
    if (pretty)
      ss << "\n";
    const auto &obj = AsObject(node);
    size_t i = 0;
    for (const auto &[key, val] : obj) {
      if (pretty)
        ss << std::string((depth + 1) * 2, ' ');
      ss << "\"" << key << "\": ";
      StringifyValue(val, ss, pretty, depth + 1);
      if (i < obj.size() - 1)
        ss << ",";
      if (pretty)
        ss << "\n";
      i++;
    }
    if (pretty)
      ss << std::string(depth * 2, ' ');
    ss << "}";
  }
}

} // namespace

bool IsNull(const Node &node) {
  return std::holds_alternative<Null>(node.value);
}
bool IsBool(const Node &node) {
  return std::holds_alternative<Bool>(node.value);
}
bool IsNumber(const Node &node) {
  return std::holds_alternative<Number>(node.value);
}
bool IsString(const Node &node) {
  return std::holds_alternative<String>(node.value);
}
bool IsArray(const Node &node) {
  return std::holds_alternative<Array>(node.value);
}
bool IsObject(const Node &node) {
  return std::holds_alternative<Object>(node.value);
}

bool AsBool(const Node &node, bool def) {
  if (IsBool(node))
    return std::get<Bool>(node.value);
  return def;
}
double AsNumber(const Node &node, double def) {
  if (IsNumber(node))
    return std::get<Number>(node.value);
  return def;
}
std::string AsString(const Node &node, const std::string &def) {
  if (IsString(node))
    return std::get<String>(node.value);
  return def;
}
const Array &AsArray(const Node &node) {
  static Array empty;
  if (IsArray(node))
    return std::get<Array>(node.value);
  return empty;
}
const Object &AsObject(const Node &node) {
  static Object empty;
  if (IsObject(node))
    return std::get<Object>(node.value);
  return empty;
}
Array &AsArray(Node &node) {
  if (!IsArray(node))
    node.value = Array{};
  return std::get<Array>(node.value);
}
Object &AsObject(Node &node) {
  if (!IsObject(node))
    node.value = Object{};
  return std::get<Object>(node.value);
}

Node Parse(const std::string &json) {
  LexerState state{json, 0};
  Token tok = NextToken(state);
  return ParseValue(state, tok);
}

std::string Stringify(const Node &node, bool pretty) {
  std::stringstream ss;
  StringifyValue(node, ss, pretty, 0);
  return ss.str();
}

} // namespace Util::JSON
