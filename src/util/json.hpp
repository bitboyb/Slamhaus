#pragma once

#ifndef UTIL_JSON_HPP
#define UTIL_JSON_HPP

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Util::JSON {

/// Represents a JSON value which can be null, boolean, number, string, array,
/// or object.
struct Node;

using Null = std::monostate;
using Bool = bool;
using Number = double;
using String = std::string;
using Array = std::vector<Node>;
using Object = std::map<std::string, Node>;

/// Container for arbitrary JSON data.
struct Node {
  std::variant<Null, Bool, Number, String, Array, Object> value;
};

// -----------------------------------------------------------------------------
// Type Checking
// -----------------------------------------------------------------------------

/// Checks if the node is null.
bool IsNull(const Node &node);

/// Checks if the node is a boolean.
bool IsBool(const Node &node);

/// Checks if the node is a number.
bool IsNumber(const Node &node);

/// Checks if the node is a string.
bool IsString(const Node &node);

/// Checks if the node is an array.
bool IsArray(const Node &node);

/// Checks if the node is an object.
bool IsObject(const Node &node);

// -----------------------------------------------------------------------------
// Value Access
// -----------------------------------------------------------------------------

/// Returns the boolean value or default if not a boolean.
bool AsBool(const Node &node, bool def = false);

/// Returns the number value or default if not a number.
double AsNumber(const Node &node, double def = 0.0);

/// Returns the string value or default if not a string.
std::string AsString(const Node &node, const std::string &def = "");

/// Returns a reference to the array or empty if not an array.
const Array &AsArray(const Node &node);

/// Returns a reference to the object or empty if not an object.
const Object &AsObject(const Node &node);

/// Returns a reference to the array allowing modification.
Array &AsArray(Node &node);

/// Returns a reference to the object allowing modification.
Object &AsObject(Node &node);

// -----------------------------------------------------------------------------
// Parsing & Serialization
// -----------------------------------------------------------------------------

/// Parses a JSON string into a Node.
/// @param json The JSON string to parse.
/// @return The parsed JSON Node. Returns a Null node on failure.
Node Parse(const std::string &json);

/// Serializes a Node into a JSON string.
/// @param node The JSON Node to serialize.
/// @param pretty If true, formats the output with indentation.
/// @return The serialized JSON string.
std::string Stringify(const Node &node, bool pretty = false);

} // namespace Util::JSON

#endif // UTIL_JSON_HPP
