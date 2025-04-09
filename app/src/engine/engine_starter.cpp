#include "engine/engine_starter.hpp"

#include "engine/abstract_engine.hpp"
#include "engine/kedro.hpp"

#include "data/settings.hpp"

std::unique_ptr<AbstractEngine> EngineStarter::init()
{
    auto engine = data::Settings::instance().value("engine").toString().toLower();
    if (engine == "kedro")
        return std::make_unique<Kedro>();
    qWarning() << "Engine can't be found, defaulting to Kedro";
    return std::make_unique<Kedro>();
}