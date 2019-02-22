#include "gcodelib/parser/Scanner.h"
#include <iostream>
#include <string>
#include <regex>

namespace GCodeLib {

  static std::regex Whitespaces(R"(^[\s]+)");
  static std::regex Integer(R"(^-?[0-9]+)");
  static std::regex Float(R"(^-?[0-9]+\.[0-9]+)");
  static std::regex Literal(R"(^[a-zA-Z_]{2,}[\w_]*)");
  static std::regex Operator(R"(^[GMTSPXYZUVWIJDHFRQEN*%])");
  static std::regex Comment(R"(^;.*$)");
  static std::regex BracedComment(R"(^\([^\)]*\))");

  static bool match_regex(const std::string &string, std::smatch &match, const std::regex &regex) {
    return std::regex_search(string, match, regex) && !match.empty();
  }

  GCodeDefaultScanner::GCodeDefaultScanner(std::istream &is, unsigned int options)
    : input(is), buffer(""), source_position("", 0, 0), options(options) {
    this->next_line();
  }

  std::optional<GCodeToken> GCodeDefaultScanner::next() {
    std::optional<GCodeToken> token;
    while (!token.has_value() && !this->finished()) {
      token = this->raw_next();
      if (token.has_value()) {
        if (((this->options & GCodeDefaultScanner::FilterEnd) != 0 &&
          token.value().is(GCodeToken::Type::End)) ||
          ((this->options & GCodeDefaultScanner::FilterComments) != 0 &&
          token.value().is(GCodeToken::Type::Comment))) {
          token.reset();
        }
      }
    }
    return token;
  }

  bool GCodeDefaultScanner::finished() {
    return this->buffer.empty() && !this->input.good();
  }

  std::optional<GCodeToken> GCodeDefaultScanner::raw_next() {
    if (this->finished()) {
      return std::optional<GCodeToken>();
    }
    this->skipWhitespaces();

    if (this->buffer.empty()) {
      GCodeToken token(this->source_position);
      this->next_line();
      return token;
    }
    std::smatch match;
    std::optional<GCodeToken> token;
    if (match_regex(this->buffer, match, Float)) {
      token = GCodeToken(std::stod(match.str()), this->source_position);
    } else if (match_regex(this->buffer, match, Integer)) {
      token = GCodeToken(static_cast<int64_t>(std::stoll(match.str())), this->source_position); 
    } else if (match_regex(this->buffer, match, Literal)) {
      token = GCodeToken(match.str(), true, this->source_position);
    } else if (match_regex(this->buffer, match, Operator)) {
      char chr = std::toupper(match.str()[0]);
      token = GCodeToken(static_cast<GCodeOperator>(chr), this->source_position);
    } else if (match_regex(this->buffer, match, Comment) || match_regex(this->buffer, match, BracedComment)) {
      token = GCodeToken(match.str(), false, this->source_position);
    }

    if (!match.empty()) {
      this->shift(match.length());
    } else {
      this->shift(1);
    }
    return token;
  }

  void GCodeDefaultScanner::next_line() {
    this->buffer.clear();
    if (this->input.good()) {
      std::getline(this->input, this->buffer);
      this->source_position.update(this->source_position.getLine() + 1, 1);
    }
  }

  void GCodeDefaultScanner::shift(std::size_t len) {
    this->buffer.erase(0, len);
    this->source_position.update(this->source_position.getLine(), this->source_position.getColumn() + len);
  }

  void GCodeDefaultScanner::skipWhitespaces() {
    std::smatch match;
    if (match_regex(this->buffer, match, Whitespaces)) {
      this->shift(match.length());
    }
  }
}