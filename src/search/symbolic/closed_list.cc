#include "closed_list.h"

#include "sym_solution_registry.h"
#include "sym_state_space_manager.h"
#include "sym_utils.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace symbolic {

ClosedList::ClosedList() : mgr(nullptr) {}

void ClosedList::init(SymStateSpaceManager *manager,
                      UnidirectionalSearch *search) {
  mgr = manager;
  my_search = search;
  map<int, vector<BDD>>().swap(zeroCostClosed);
  map<int, BDD>().swap(closed);
  closedTotal = mgr->zeroBDD();
}

void ClosedList::init(SymStateSpaceManager *manager,
                      UnidirectionalSearch *search, const ClosedList &other) {
  mgr = manager;
  my_search = search;
  map<int, vector<BDD>>().swap(zeroCostClosed);
  map<int, BDD>().swap(closed);
  closedTotal = mgr->zeroBDD();

  closedTotal = other.closedTotal;
  closed[0] = closedTotal;
}

void ClosedList::insert(int h, const BDD &S) {
  if (closed.count(h)) {
    assert(h_values.count(h));
    closed[h] += S;
  } else {
    closed[h] = S;
  }

  if (mgr->hasTransitions0()) {
    zeroCostClosed[h].push_back(S);
  }
  closedTotal += S;
}

BDD ClosedList::getPartialClosed(int upper_bound) const {
  BDD res = mgr->zeroBDD();
  for (const auto &pair : closed) {
    if (pair.first > upper_bound) {
      break;
    }
    res += pair.second;
  }
  return res;
}

std::vector<SymSolutionCut> ClosedList::getAllCuts(const BDD &states, int g,
                                                   bool fw,
                                                   int lower_bound) const {
  std::vector<SymSolutionCut> result;
  BDD cut_candidate = states * closedTotal;
  if (!cut_candidate.IsZero()) {
    for (const auto &closedH : closed) {
      int h = closedH.first;

      /* Here we also need to consider higher costs due to the architecture
       of symBD. Otherwise their occur problems in
       */
      if (g + h < lower_bound) {
        continue;
      }

      // cout << "Check cut of g=" << g << " with h=" << h << endl;
      BDD cut = closedH.second * cut_candidate;
      if (!cut.IsZero()) {
        if (fw) {
          result.emplace_back(g, h, cut);
        } else {
          result.emplace_back(h, g, cut);
        }
      }
    }
  }
  return result;
}

} // namespace symbolic
