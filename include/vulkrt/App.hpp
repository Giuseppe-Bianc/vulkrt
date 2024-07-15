//
// Created by gbian on 15/07/2024.
//

#pragma once

#include "Window.hpp"

namespace lve {

class App {
public:
  void run();
private:
  Window lveWindow{WWIDTH, WHEIGHT, WTITILE};
};

} // lve
