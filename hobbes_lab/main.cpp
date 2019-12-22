#include <iostream>
#include <stdexcept>
#include <hobbes/hobbes.H>

int main() {
  hobbes::cc c;

  while (std::cin) {
    std::cout << "> " << std::flush;

    std::string line;
    std::getline(std::cin, line);
    if (line == ":q") break;

    try {
      c.compileFn<void()>("print(" + line + ")")();
    } catch (std::exception& ex) {
      std::cout << "*** " << ex.what();
    }

    std::cout << std::endl;
    hobbes::resetMemoryPool();
  }

  return 0;
}
