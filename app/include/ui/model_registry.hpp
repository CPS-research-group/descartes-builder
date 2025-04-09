#pragma once

#include <QtNodes/NodeDelegateModelRegistry>

#include "ui/models/coder_models.hpp"
#include "ui/models/io_models.hpp"
#include "ui/models/processor_models.hpp"
#include "ui/models/trainer_models.hpp"

namespace model_registry {
static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels()
{
    // All models to be used must be registered here
    auto ret = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    ret->registerModel<DataSourceModel>("IO");
    ret->registerModel<FuncOutModel>("IO");
    ret->registerModel<DataOutModel>("IO");
    ret->registerModel<GraphModel>("IO");
    ret->registerModel<SplitDataModel>("Processor");
    ret->registerModel<ScoreModel>("Processor");
    ret->registerModel<SensitivityAnalysisModel>("Processor");
    ret->registerModel<DifferenceModel>("Processor");
    ret->registerModel<ExternalProcessorModel>("Processor");
    ret->registerModel<TransformDataModel>("Coder");
    ret->registerModel<ReduceDataModel>("Coder");
    ret->registerModel<BasicTrainerModel>("Trainer");
    ret->registerModel<TorchTrainerModel>("Trainer");

    return ret;
}
} // namespace model_registry
