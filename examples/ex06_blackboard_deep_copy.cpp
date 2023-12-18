#include "behaviortree_cpp/bt_factory.h"
#include "dummy_nodes.h"

using namespace BT;

// clang-format off
static const char* xml_tree = R"(
<root BTCPP_format="4">

  <BehaviorTree ID="MainTree">
    <Sequence>
      <Script code="val_A:= 'john' "/>
      <Script code="val_B:= 42 "/>
      <SaySomething message="{val_A}" />
      <SaySomething message="hello world" />
      <SubTree ID="Sub" val="{val_A}" _autoremap="true" />
      <SaySomething message="{reply}" />
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="Sub">
    <Sequence>
      <SaySomething message="{val}" />
      <SaySomething message="{val_B}" />
      <Script code="reply:= 'done' "/>
    </Sequence>
  </BehaviorTree>

</root>
 )";

// clang-format on


void BlackboardClone(const Blackboard& src, Blackboard& dst)
{
  dst.clear();
  for(auto const key_name: src.getKeys())
  {
    const auto key = std::string(key_name);
    const auto entry = src.getEntry(key);
    dst.createEntry(key, entry->info);
    auto new_entry =  dst.getEntry(key);
    new_entry->value = entry->value;
    new_entry->string_converter = entry->string_converter;
  }
}

std::vector<Blackboard::Ptr> BlackboardBackup(const BT::Tree& tree)
{
  std::vector<Blackboard::Ptr> bb;
  bb.reserve(tree.subtrees.size());
  for(const auto& sub: tree.subtrees)
  {
    bb.push_back( BT::Blackboard::create() );
    BlackboardClone(*sub->blackboard, *bb.back());
  }
  return bb;
}


void BlackboardRestore(const std::vector<Blackboard::Ptr>& backup, BT::Tree& tree)
{
  assert(backup.size() == tree.subtrees.size());
  for(size_t i=0; i<tree.subtrees.size(); i++)
  {
    BlackboardClone(*(backup[i]), *(tree.subtrees[i]->blackboard));
  }
}


int main()
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerBehaviorTreeFromText(xml_tree);

  auto tree = factory.createTree("MainTree");

  const auto backup = BlackboardBackup(tree);
  tree.tickWhileRunning();

  // execute again as it was the first time.
  BlackboardRestore(backup, tree);
  tree.tickWhileRunning();

  return 0;
}
