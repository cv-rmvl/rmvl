#include <csignal>

#include "rmvl/opcua/server.hpp"

using namespace rm;

Server server(4840);

static inline void onHandle(int) { server.shutdown(); }

int main()
{
    signal(SIGINT, onHandle);

    Variable value_1 = 42;
    value_1.display_name = "Value 1";
    value_1.browse_name = "value_1";
    Variable value_2 = 3.14;
    value_2.display_name = "Value 2";
    value_2.browse_name = "value_2";
    Method add = [](ServerView, const NodeId &, InputVariables iargs, OutputVariables oargs) -> bool {
        int num1 = iargs[0], num2 = iargs[1];
        oargs[0] = num1 + num2;
        return true;
    };
    add.display_name = "Add";
    add.browse_name = "add";
    add.iargs = {{"num1", tpInt32}, {"num2", tpInt32}};
    add.oargs = {{"result", tpInt32}};
    server.addVariableNode(value_1);
    server.addVariableNode(value_2);
    server.addMethodNode(add);
    printf("节点信息:\n");
    printf("  ObjectFolders:\n");
    printf("  - value_1:\n");
    printf("      node: Variable\n");
    printf("      value: 42\n");
    printf("      type: Int32\n");
    printf("  - value_2:\n");
    printf("      node: Variable\n");
    printf("      value: 3.14\n");
    printf("      type: Double\n");
    printf("  - add:\n");
    printf("      node: Method\n");
    printf("      input: num1(Int32), num2(Int32)\n");
    printf("      output: result(Int32)\n");
    server.spin();
    return 0;
}
