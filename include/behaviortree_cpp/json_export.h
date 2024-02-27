#pragma once

#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/blackboard.h"

// Use the version nlohmann::json embedded in BT.CPP
#include "behaviortree_cpp/contrib/json.hpp"

namespace BT {

/**
*  To add new type, you must follow these isntructions:
*    https://json.nlohmann.me/features/arbitrary_types/
*
*  Considering for instance the type:
*
*   struct Point2D {
*     double x;
*     double y;
*   };
*
*  This would require the implementation of:
*
*   void to_json(nlohmann::json& j, const Point2D& point);
*   void from_json(const nlohmann::json& j, Point2D& point);
*
*  To avoir repeating yourself, we provide the macro BT_JSON_CONVERTION
*  that implements both those function, at once.
*  Usage:
*
*  BT_JSON_CONVERTER(Point2D, point)
*  {
*     add_field("x", &point.x);
*     add_field("y", &point.y);
*  }
*
*  Later, you MUST register the type in main using:
*
*   RegisterJsonDefinition<Point2D>();
*/

//-----------------------------------------------------------------------------------

/**
*  Use RegisterJsonDefinition<Foo>();
*/

class JsonExporter {

  public:
  static JsonExporter& get() {
    static JsonExporter global_instance;
    return global_instance;
  }

  /**
   * @brief toJson adds the content of "any" to the JSON "destination".
   *
   * It will return false if the conversion toJson is not possible
   * ( you might need to register the converter with addConverter() ).
   */
  bool toJson(const BT::Any& any, nlohmann::json& destination) const;

  template <typename T>
  void toJson(const T& val, nlohmann::json& dst) const {
    dst = val;
  }

  /// Register new JSON converters with addConverter<Foo>().
  /// You should have used first the macro BT_JSON_CONVERTER
  template <typename T> void addConverter();

private:

  using ToJonConverter = std::function<void(const BT::Any&, nlohmann::json&)>;
  using FromJonConverter = std::function<BT::Any(const nlohmann::json&)>;

  std::unordered_map<std::type_index, ToJonConverter> to_json_converters_;
  std::unordered_map<std::string, FromJonConverter> from_json_converters_;
};

//-------------------------------------------------------------------

template<typename T> inline
void JsonExporter::addConverter()
{
  ToJonConverter to_converter = [](const BT::Any& entry, nlohmann::json& dst)
  {
    using namespace nlohmann;
    to_json(dst, *const_cast<BT::Any&>(entry).castPtr<T>());
  };
  to_json_converters_.insert( {typeid(T), to_converter} );

  FromJonConverter from_converter = [](const nlohmann::json& dst) -> BT::Any
  {
    T value;
    using namespace nlohmann;
    from_json(dst, value);
    return BT::Any(value);
  };

  // we need to get the name of the type
  nlohmann::json const js = T{};
  auto const type_name = js.contains("__type") ?
                             std::string(js["__type"]) :
                             BT::demangle(typeid(T));

  from_json_converters_.insert( {type_name, from_converter} );
}

template <typename T> inline void RegisterJsonDefinition()
{
  JsonExporter::get().addConverter<T>();
}


nlohmann::json ExportBlackboardToJSON(BT::Blackboard& blackboard);

} // namespace BT

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

// Macro to implement to_json() and from_json()

#define BT_JSON_CONVERTER(Type, value) \
template <class AddField> void _JsonTypeDefinition(Type&, AddField&); \
\
inline void to_json(nlohmann::json& js, const Type& p, bool add_type_name = false) { \
  auto op = [&js](const char* name, auto* val) { to_json(js[name], *val); }; \
  _JsonTypeDefinition(const_cast<Type&>(p), op); \
  js["__type"] = #Type; \
} \
\
inline void from_json(const nlohmann::json& js, Type& p) { \
  auto op = [&js](const char* name, auto* v) { js.at(name).get_to(*v); }; \
  _JsonTypeDefinition(p, op); \
} \
\
template <class AddField> inline \
void _JsonTypeDefinition(Type& value, AddField& add_field)\

