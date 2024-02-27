#include <gtest/gtest.h>
#include "behaviortree_cpp/json_export.h"

//----------- Custom types ----------

namespace TestTypes {

struct Vector3D {
  double x;
  double y;
  double z;
};

struct Quaternion3D {
  double w;
  double x;
  double y;
  double z;
};

struct Pose3D {
  Vector3D pos;
  Quaternion3D rot;
};

BT_JSON_CONVERTER(Vector3D, v)
{
  add_field("x", &v.x);
  add_field("y", &v.y);
  add_field("z", &v.z);
}

BT_JSON_CONVERTER(Quaternion3D, v)
{
  add_field("w", &v.w);
  add_field("x", &v.x);
  add_field("y", &v.y);
  add_field("z", &v.z);
}

BT_JSON_CONVERTER(Pose3D, v)
{
  add_field("pos", &v.pos);
  add_field("rot", &v.rot);
}

} // namespace TestTypes


//----------- JSON specialization ----------


TEST(JsonTest, Exporter)
{
  BT::JsonExporter exporter;

  TestTypes::Pose3D pose = { {1,2,3},
                            {4,5,6,7} };

  nlohmann::json json;
  exporter.toJson(BT::Any(69), json["int"]);
  exporter.toJson(BT::Any(3.14), json["real"]);

  // expected to throw, because we haven't called addConverter()
  ASSERT_FALSE( exporter.toJson(BT::Any(pose), json["pose"]) );

  // now it should work
  exporter.addConverter<TestTypes::Pose3D>();
  exporter.toJson(BT::Any(pose), json["pose"]);

  std::cout << json.dump(2) << std::endl;

  ASSERT_EQ(json["int"],69);
  ASSERT_EQ(json["real"], 3.14);

  ASSERT_EQ(json["pose"]["__type"], "Pose3D");
  ASSERT_EQ(json["pose"]["pos"]["x"], 1);
  ASSERT_EQ(json["pose"]["pos"]["y"], 2);
  ASSERT_EQ(json["pose"]["pos"]["z"], 3);

  ASSERT_EQ(json["pose"]["rot"]["w"], 4);
  ASSERT_EQ(json["pose"]["rot"]["x"], 5);
  ASSERT_EQ(json["pose"]["rot"]["y"], 6);
  ASSERT_EQ(json["pose"]["rot"]["z"], 7);
}


