#pragma once

#include <memory>

#include "engine/abstract_engine.hpp"

class EngineStarter
{
public:
    static std::unique_ptr<AbstractEngine> init();
};
