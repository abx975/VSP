#include <string>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

using std::cout;
using std::cerr;
using namespace caf;

using exp_atom = atom_constant<atom("exp")>;
using add_atom = atom_constant<atom("add")>;
using sub_atom = atom_constant<atom("sub")>;
using mul_atom = atom_constant<atom("mul")>;
using div_atom = atom_constant<atom("div")>;

behavior math() {
  return {
    [](exp_atom, int x){
	int res=1;
	while(x>0){
		res*=2;	
		cerr << "res=" << res << " ";
		x--;
	}
      return res;
    },
    [](add_atom, int x, int y) {
      return x + y;
    },
    [](sub_atom, int x, int y) {
      return x - y;
    },
    [](mul_atom, int x, int y) {
      return x * y;
    },
    [](div_atom, int x, int y) -> result<int> {
      if (y == 0)
        return sec::invalid_argument;
      return x / y;
    }
  };
}

std::ostream& operator<<(std::ostream& out, const expected<message>& x) {
  return out << to_string(x);
}

struct config : actor_system_config {
  std::string host = "localhost";
  uint16_t port = 9900;
  bool server = true;
  config() {
    opt_group{custom_options_, "global"}
    .add(host, "host,H", "hostname of server")
    .add(port, "port,p", "IP port for publish/remote_actor")
    .add(server, "server,s", "run as server");
  }
};

void caf_main(actor_system& sys, const config& cfg) {
  auto& mm = sys.middleman();
  auto self =sys.spawn(math);
  if (cfg.server) {
    auto p = mm.publish(self, cfg.port);
    if (!p)
      cerr << "unable to publish actor: " << sys.render(p.error()) << "\n";
    else
      cout << "math actor published at port " << *p << "\n";
  } else {
    auto x = mm.remote_actor(cfg.host, cfg.port);
    if (!x)
      cerr << "unable to connect to server: " << sys.render(x.error()) << "\n";
    else {
	//self->send(*x,1);
      auto f = make_function_view(*x);
      cout << "f('add', 4, 2) = " << f(add_atom::value, 4, 2) << "\n"
           << "f('sub', 4, 2) = " << f(sub_atom::value, 4, 2) << "\n"
           << "f('mul', 4, 2) = " << f(mul_atom::value, 4, 2) << "\n"
           << "f('div', 4, 2) = " << f(div_atom::value, 4, 2) << "\n"
           << "f('div', 1, 0) = " << f(div_atom::value, 1, 0) << "\n";
    }
  }
}

CAF_MAIN(io::middleman)

