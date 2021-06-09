#include "cost_adapted_task.h"

#include "../operator_cost.h"
#include "../option_parser.h"
#include "../plugin.h"

#include "../task_utils/task_properties.h"
#include "../tasks/root_task.h"
#include "../utils/system.h"

#include <iostream>
#include <memory>

using namespace std;
using utils::ExitCode;

namespace tasks {
CostAdaptedTask::CostAdaptedTask(
    const shared_ptr<AbstractTask> &parent,
    OperatorCost cost_type)
    : DelegatingTask(parent),
      cost_type(cost_type),
      parent_is_unit_cost(task_properties::is_unit_cost(TaskProxy(*parent))) {
}

int CostAdaptedTask::get_operator_cost(int index, bool is_axiom) const {
    OperatorProxy op(*parent, index, is_axiom);
    return get_adjusted_action_cost(op, cost_type, parent_is_unit_cost);
}

string CostAdaptedTask::get_operator_cost_function(int index, bool is_axiom) const {
    if (!is_axiom) {
        bool sdac = parent->get_operator_cost_function(index, false).find_first_not_of("0123456789");
        if (sdac) {
            switch (cost_type) {
            case NORMAL:
                return parent->get_operator_cost_function(index, false);
            case ONE:
                return "1";
            case PLUSONE:
                if (parent_is_unit_cost)
                    return "1";
                else
                    return "(" + parent->get_operator_cost_function(index, false) + " + 1)";
            default:
                ABORT("Unknown cost type");
            }
        }
    }
    return to_string(get_operator_cost(index, is_axiom));
}


static shared_ptr<AbstractTask> _parse(OptionParser &parser) {
    parser.document_synopsis(
        "Cost-adapted task",
        "A cost-adapting transformation of the root task.");
    add_cost_type_option_to_parser(parser);
    Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    } else {
        OperatorCost cost_type = OperatorCost(opts.get_enum("cost_type"));
        return make_shared<CostAdaptedTask>(g_root_task, cost_type);
    }
}

static Plugin<AbstractTask> _plugin("adapt_costs", _parse);
}
