#pragma once

#include <QString>

#include <QtUtility/data/constexpr_qstring.hpp>

using ConstLatin1String = QtUtility::data::ConstLatin1String;

namespace processor_function {

constexpr ConstLatin1String PROCESSOR = ""; // the first input must be a function
constexpr ConstLatin1String SPLIT_DATA = "split_data";
constexpr ConstLatin1String DIFFERENCE = "difference";
constexpr ConstLatin1String SCORE = "score";
constexpr ConstLatin1String LOAD_MAT = "load_mat";
constexpr ConstLatin1String SENSITIVITY_ANALYSIS = "sensitivity_analysis";

} // namespace processor_function

namespace coder_function {

constexpr ConstLatin1String TRANSFORM_DATA = "xform_data";
constexpr ConstLatin1String REDUCE_DATA = "reduce_data";

} // namespace coder_function

namespace trainer_function {

constexpr ConstLatin1String BASIC_TRAINER = "basic_trainer";
constexpr ConstLatin1String PYTORCH_TRAINER = "pytorch_trainer";

} // namespace trainer_function
