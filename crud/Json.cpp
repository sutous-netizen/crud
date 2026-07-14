#include "Json.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include <cctype>
#include <cstdio>

namespace json {

    namespace {

        class Parser {
        public:
            explicit Parser(const std::string& text) : text_(text) {}

            Value Parse() {
                SkipWhitespace();
                Value result = ParseValue();
                SkipWhitespace();
                if (pos_ != text_.size()) {
                    Fail("Unexpected trailing content after JSON value");
                }
                return result;
            }

        private:
            const std::string& text_;
            size_t pos_ = 0;
            size_t line_ = 1;
            size_t column_ = 1;

            [[noreturn]] void Fail(const std::string& message) {
                throw ParseException(message, line_, column_);
            }

            bool AtEnd() const { return pos_ >= text_.size(); }

            char Peek() const {
                if (AtEnd()) {
                    return '\0';
                }
                return text_[pos_];
            }

            char Advance() {
                char c = text_[pos_++];
                if (c == '\n') {
                    ++line_;
                    column_ = 1;
                } else {
                    ++column_;
                }
                return c;
            }

            void SkipWhitespace() {
                while (!AtEnd()) {
                    char c = Peek();
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                        Advance();
                    } else {
                        break;
                    }
                }
            }

            void Expect(char expected) {
                if (AtEnd() || Peek() != expected) {
                    Fail(std::string("Expected '") + expected + "'");
                }
                Advance();
            }

            bool Consume(const char* literal) {
                size_t len = std::char_traits<char>::length(literal);
                if (pos_ + len > text_.size()) {
                    return false;
                }
                if (text_.compare(pos_, len, literal) != 0) {
                    return false;
                }
                for (size_t i = 0; i < len; ++i) {
                    Advance();
                }
                return true;
            }

            Value ParseValue() {
                SkipWhitespace();
                if (AtEnd()) {
                    Fail("Unexpected end of input");
                }
                switch (Peek()) {
                case '{': return ParseObject();
                case '[': return ParseArray();
                case '"': return Value(ParseString());
                case 't':
                case 'f': return ParseBool();
                case 'n': return ParseNull();
                default: return ParseNumber();
                }
            }

            Value ParseObject() {
                Expect('{');
                Value::Object members;
                SkipWhitespace();
                if (Peek() == '}') {
                    Advance();
                    return Value(std::move(members));
                }
                while (true) {
                    SkipWhitespace();
                    if (Peek() != '"') {
                        Fail("Expected string key in object");
                    }
                    std::string key = ParseString();
                    SkipWhitespace();
                    Expect(':');
                    Value value = ParseValue();
                    members.emplace_back(std::move(key), std::move(value));
                    SkipWhitespace();
                    if (Peek() == ',') {
                        Advance();
                        continue;
                    }
                    break;
                }
                SkipWhitespace();
                Expect('}');
                return Value(std::move(members));
            }

            Value ParseArray() {
                Expect('[');
                Value::Array elements;
                SkipWhitespace();
                if (Peek() == ']') {
                    Advance();
                    return Value(std::move(elements));
                }
                while (true) {
                    elements.push_back(ParseValue());
                    SkipWhitespace();
                    if (Peek() == ',') {
                        Advance();
                        continue;
                    }
                    break;
                }
                SkipWhitespace();
                Expect(']');
                return Value(std::move(elements));
            }

            std::string ParseString() {
                Expect('"');
                std::string result;
                while (true) {
                    if (AtEnd()) {
                        Fail("Unterminated string literal");
                    }
                    char c = Advance();
                    if (c == '"') {
                        break;
                    }
                    if (c == '\\') {
                        if (AtEnd()) {
                            Fail("Unterminated escape sequence");
                        }
                        char escaped = Advance();
                        switch (escaped) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/': result += '/'; break;
                        case 'b': result += '\b'; break;
                        case 'f': result += '\f'; break;
                        case 'n': result += '\n'; break;
                        case 'r': result += '\r'; break;
                        case 't': result += '\t'; break;
                        case 'u': result += ParseUnicodeEscape(); break;
                        default: Fail("Invalid escape sequence");
                        }
                    } else {
                        result += c;
                    }
                }
                return result;
            }

            // Decodes \uXXXX (including surrogate pairs) into UTF-8 bytes.
            std::string ParseUnicodeEscape() {
                uint32_t codepoint = ParseHex4();
                if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                    if (Peek() == '\\') {
                        size_t savedPos = pos_, savedLine = line_, savedColumn = column_;
                        Advance();
                        if (Peek() == 'u') {
                            Advance();
                            uint32_t low = ParseHex4();
                            if (low >= 0xDC00 && low <= 0xDFFF) {
                                codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                            } else {
                                pos_ = savedPos; line_ = savedLine; column_ = savedColumn;
                            }
                        } else {
                            pos_ = savedPos; line_ = savedLine; column_ = savedColumn;
                        }
                    }
                }
                return EncodeUtf8(codepoint);
            }

            uint32_t ParseHex4() {
                uint32_t value = 0;
                for (int i = 0; i < 4; ++i) {
                    if (AtEnd()) {
                        Fail("Invalid \\u escape sequence");
                    }
                    char c = Advance();
                    value <<= 4;
                    if (c >= '0' && c <= '9') value |= (c - '0');
                    else if (c >= 'a' && c <= 'f') value |= (c - 'a' + 10);
                    else if (c >= 'A' && c <= 'F') value |= (c - 'A' + 10);
                    else Fail("Invalid hex digit in \\u escape sequence");
                }
                return value;
            }

            static std::string EncodeUtf8(uint32_t codepoint) {
                std::string out;
                if (codepoint <= 0x7F) {
                    out += static_cast<char>(codepoint);
                } else if (codepoint <= 0x7FF) {
                    out += static_cast<char>(0xC0 | (codepoint >> 6));
                    out += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else if (codepoint <= 0xFFFF) {
                    out += static_cast<char>(0xE0 | (codepoint >> 12));
                    out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    out += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else {
                    out += static_cast<char>(0xF0 | (codepoint >> 18));
                    out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                    out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    out += static_cast<char>(0x80 | (codepoint & 0x3F));
                }
                return out;
            }

            Value ParseBool() {
                if (Consume("true")) return Value(true);
                if (Consume("false")) return Value(false);
                Fail("Invalid literal");
            }

            Value ParseNull() {
                if (Consume("null")) return Value(nullptr);
                Fail("Invalid literal");
            }

            Value ParseNumber() {
                size_t start = pos_;
                if (Peek() == '-') Advance();
                if (AtEnd() || !std::isdigit(static_cast<unsigned char>(Peek()))) {
                    Fail("Invalid number");
                }
                if (Peek() == '0') {
                    Advance();
                } else {
                    while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) Advance();
                }
                if (!AtEnd() && Peek() == '.') {
                    Advance();
                    if (AtEnd() || !std::isdigit(static_cast<unsigned char>(Peek()))) {
                        Fail("Invalid number: expected digit after decimal point");
                    }
                    while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) Advance();
                }
                if (!AtEnd() && (Peek() == 'e' || Peek() == 'E')) {
                    Advance();
                    if (!AtEnd() && (Peek() == '+' || Peek() == '-')) Advance();
                    if (AtEnd() || !std::isdigit(static_cast<unsigned char>(Peek()))) {
                        Fail("Invalid number: expected digit in exponent");
                    }
                    while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) Advance();
                }
                std::string token = text_.substr(start, pos_ - start);
                return Value(std::stod(token));
            }
        };

        void AppendEscapedString(std::string& out, const std::string& value) {
            out += '"';
            for (unsigned char c : value) {
                switch (c) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\b': out += "\\b"; break;
                case '\f': out += "\\f"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if (c < 0x20) {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                        out += buf;
                    } else {
                        out += static_cast<char>(c);
                    }
                }
            }
            out += '"';
        }

        std::string FormatNumber(double value) {
            if (std::isfinite(value) && value == static_cast<long long>(value) &&
                std::abs(value) < 1e15) {
                return std::to_string(static_cast<long long>(value));
            }
            std::ostringstream oss;
            oss.precision(17);
            oss << value;
            return oss.str();
        }

    } // namespace

    bool Value::AsBool() const {
        if (type_ != ValueType::Boolean) {
            throw std::runtime_error("json::Value is not a boolean");
        }
        return bool_;
    }

    double Value::AsNumber() const {
        if (type_ != ValueType::Number) {
            throw std::runtime_error("json::Value is not a number");
        }
        return number_;
    }

    const std::string& Value::AsString() const {
        if (type_ != ValueType::String) {
            throw std::runtime_error("json::Value is not a string");
        }
        return string_;
    }

    const Value::Array& Value::AsArray() const {
        if (type_ != ValueType::Array) {
            throw std::runtime_error("json::Value is not an array");
        }
        return array_;
    }

    Value::Array& Value::AsArray() {
        if (type_ != ValueType::Array) {
            throw std::runtime_error("json::Value is not an array");
        }
        return array_;
    }

    const Value::Object& Value::AsObject() const {
        if (type_ != ValueType::Object) {
            throw std::runtime_error("json::Value is not an object");
        }
        return object_;
    }

    Value::Object& Value::AsObject() {
        if (type_ != ValueType::Object) {
            throw std::runtime_error("json::Value is not an object");
        }
        return object_;
    }

    const Value& Value::operator[](const std::string& key) const {
        if (type_ != ValueType::Object) {
            throw std::runtime_error("json::Value is not an object");
        }
        for (const auto& member : object_) {
            if (member.first == key) {
                return member.second;
            }
        }
        throw std::runtime_error("json::Value object has no member '" + key + "'");
    }

    Value& Value::operator[](const std::string& key) {
        if (type_ == ValueType::Null) {
            type_ = ValueType::Object;
        }
        if (type_ != ValueType::Object) {
            throw std::runtime_error("json::Value is not an object");
        }
        for (auto& member : object_) {
            if (member.first == key) {
                return member.second;
            }
        }
        object_.emplace_back(key, Value());
        return object_.back().second;
    }

    const Value& Value::operator[](size_t index) const {
        return AsArray().at(index);
    }

    Value& Value::operator[](size_t index) {
        return AsArray().at(index);
    }

    bool Value::Contains(const std::string& key) const {
        if (type_ != ValueType::Object) {
            return false;
        }
        for (const auto& member : object_) {
            if (member.first == key) {
                return true;
            }
        }
        return false;
    }

    void Value::PushBack(Value value) {
        if (type_ == ValueType::Null) {
            type_ = ValueType::Array;
        }
        if (type_ != ValueType::Array) {
            throw std::runtime_error("json::Value is not an array");
        }
        array_.push_back(std::move(value));
    }

    void Value::Set(const std::string& key, Value value) {
        (*this)[key] = std::move(value);
    }

    void Value::WriteTo(std::string& out, int indent, int currentIndent) const {
        auto writeNewlineAndIndent = [&](int depth) {
            if (indent >= 0) {
                out += '\n';
                out.append(static_cast<size_t>(indent) * depth, ' ');
            }
        };

        switch (type_) {
        case ValueType::Null:
            out += "null";
            break;
        case ValueType::Boolean:
            out += bool_ ? "true" : "false";
            break;
        case ValueType::Number:
            out += FormatNumber(number_);
            break;
        case ValueType::String:
            AppendEscapedString(out, string_);
            break;
        case ValueType::Array: {
            if (array_.empty()) {
                out += "[]";
                break;
            }
            out += '[';
            for (size_t i = 0; i < array_.size(); ++i) {
                if (i > 0) out += ',';
                writeNewlineAndIndent(currentIndent + 1);
                array_[i].WriteTo(out, indent, currentIndent + 1);
            }
            writeNewlineAndIndent(currentIndent);
            out += ']';
            break;
        }
        case ValueType::Object: {
            if (object_.empty()) {
                out += "{}";
                break;
            }
            out += '{';
            for (size_t i = 0; i < object_.size(); ++i) {
                if (i > 0) out += ',';
                writeNewlineAndIndent(currentIndent + 1);
                AppendEscapedString(out, object_[i].first);
                out += ':';
                if (indent >= 0) out += ' ';
                object_[i].second.WriteTo(out, indent, currentIndent + 1);
            }
            writeNewlineAndIndent(currentIndent);
            out += '}';
            break;
        }
        }
    }

    std::string Value::ToString(int indent) const {
        std::string out;
        WriteTo(out, indent, 0);
        return out;
    }

    void Value::SaveToFile(const std::string& path, int indent) const {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing: " + path);
        }
        file << ToString(indent);
        if (!file) {
            throw std::runtime_error("Failed to write JSON to file: " + path);
        }
    }

    Value Value::Parse(const std::string& text) {
        Parser parser(text);
        return parser.Parse();
    }

    Value Value::ParseFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file for reading: " + path);
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return Parse(buffer.str());
    }

} // namespace json
