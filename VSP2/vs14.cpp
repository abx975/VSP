#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "caf/all.hpp"
#include "caf/io/all.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::cerr;
using std::chrono::seconds;

using namespace caf;

using cell = typed_actor<reacts_to<put_atom, int>,
                         replies_to<get_atom>::with<int>>;

struct cell_state {
  int value = 0;
};

cell::behavior_type cell_impl(cell::stateful_pointer<cell_state> self, int x0) {
  self->state.value = x0;
  return {
    [=](put_atom, int val) {
      self->state.value = val;
    },
    [=](get_atom) {
      return self->state.value;
    }
  };
}

void waiting_testee(event_based_actor* self, vector<cell> cells) {
  for (auto& x : cells)
    self->request(x, seconds(1), get_atom::value).await([=](int y) {
      aout(self) << "cell #" << x.id() << " -> " << y << endl;
    });
}

void multiplexed_testee(event_based_actor* self, vector<cell> cells) {
  for (auto& x : cells)
    self->request(x, seconds(1), get_atom::value).then([=](int y) {
      aout(self) << "cell #" << x.id() << " -> " << y << endl;
    });
}

struct config : actor_system_config {
  std::string msg = "Hello";
  config() {
    opt_group{custom_options_, "global"}
    .add(msg, "message,m", "set output");
  }
};

void caf_main(actor_system& sys, const config& cfg) {
  std::cout << cfg.msg << std::endl;

  vector<cell> cells;
  for (auto i = 0; i < 5; ++i){
    cells.emplace_back(sys.spawn(cell_impl, i * i));
  }/*
  cerr << "waiting testee" << endl;
  sys.spawn(waiting_testee,cells);
  */  
  cerr << "multiplexed_testee" << endl;
  sys.spawn(multiplexed_testee,cells);
}

CAF_MAIN(io::middleman)
