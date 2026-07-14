#pragma once

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <stdexcept>
#include <cstdint>

namespace json {

    class ParseException : public std::runtime_error {
    public:
        ParseException(const std::string& message, size_t line, size_t column)
            : std::runtime_error(message), line_(line), column_(column) {
        }

        size_t Line() const { return line_; }
        size_t Column() const { return column_; }

    private:
        size_t line_;
        size_t column_;
    };

    enum class ValueType {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object,
    };

    class Value {
    public:
        using Array = std::vector<Value>;
        using Member = std::pair<std::string, Value>;
        using Object = std::vector<Member>;

        Value() : type_(ValueType::Null) {}
        Value(std::nullptr_t) : type_(ValueType::Null) {}
        Value(bool value) : type_(ValueType::Boolean), bool_(value) {}
        Value(double value) : type_(ValueType::Number), number_(value) {}
        Value(int value) : type_(ValueType::Number), number_(static_cast<double>(value)) {}
        Value(std::string value) : type_(ValueType::String), string_(std::move(value)) {}
        Value(const char* value) : type_(ValueType::String), string_(value) {}
        Value(Array value) : type_(ValueType::Array), array_(std::move(value)) {}
        Value(Object value) : type_(ValueType::Object), object_(std::move(value)) {}

        static Value MakeArray() { return Value(Array{}); }
        static Value MakeObject() { return Value(Object{}); }

        ValueType Type() const { return type_; }

        bool IsNull() const { return type_ == ValueType::Null; }
        bool IsBoolean() const { return type_ == ValueType::Boolean; }
        bool IsNumber() const { return type_ == ValueType::Number; }
        bool IsString() const { return type_ == ValueType::String; }
        bool IsArray() const { return type_ == ValueType::Array; }
        bool IsObject() const { return type_ == ValueType::Object; }

        bool AsBool() const;
        double AsNumber() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        Array& AsArray();
        const Object& AsObject() const;
        Object& AsObject();

        // Object member access; throws if not an object or member missing.
        const Value& operator[](const std::string& key) const;
        // Object member access; inserts a Null member if missing (requires this to already be an object or null).
        Value& operator[](const std::string& key);

        // Array element access.
        const Value& operator[](size_t index) const;
        Value& operator[](size_t index);

        bool Contains(const std::string& key) const;

        void PushBack(Value value);
        void Set(const std::string& key, Value value);

        // Serializes to a JSON string. When indent >= 0, pretty-prints with that many spaces per level.
        std::string ToString(int indent = -1) const;

        // Writes the serialized JSON to a file, overwriting any existing content.
        void SaveToFile(const std::string& path, int indent = 2) const;

        // Parses JSON text into a Value. Throws ParseException on malformed input.
        static Value Parse(const std::string& text);

        // Reads and parses a JSON file. Throws std::runtime_error if the file cannot be opened.
        static Value ParseFile(const std::string& path);

    private:
        void WriteTo(std::string& out, int indent, int currentIndent) const;

        ValueType type_;
        bool bool_ = false;
        double number_ = 0.0;
        std::string string_;
        Array array_;
        Object object_;
    };

} // namespace json
