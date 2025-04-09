#pragma once

#include <QColor>

#include <QtNodes/NodeData>
#include <QtUtility/data/constexpr_qstring.hpp>

using ConstLatin1String = QtUtility::data::ConstLatin1String;

namespace constants {

// stylling constants
constexpr QColor COLOR_PRIMARY(0, 0, 0);
constexpr QColor COLOR_SECONDARY(255, 255, 255);
constexpr QColor COLOR_ACCENT(51, 153, 255);

constexpr QColor COLOR_CODER(185, 224, 165);
constexpr QColor COLOR_PROCESSOR(169, 196, 236);
constexpr QColor COLOR_TRAINER(205, 162, 190);

constexpr uint SIDE_BAR_MINIMUM_WIDTH = 250;
constexpr uint INT_LINE_EDIT_MAXIMUM_WIDTH = 150;
constexpr uint INT_SPIN_BOX_MAX_WIDTH = 65;
constexpr uint INT_SPIN_BOX_MIN_WIDTH = 60;
constexpr uint DOUBLE_SPIN_BOX_MAX_WIDTH = 80;
constexpr uint DOUBLE_SPIN_BOX_MIN_WIDTH = 75;

const uint SIDEBAR_PORT_ID_COL = 0;
const uint SIDEBAR_PORT_CAPTION_COL = 1;
const uint SIDEBAR_PORT_TYPEID_COL = 2;
const uint SIDEBAR_PORT_TYPETAG_COL = 3;

constexpr ConstLatin1String CONNECTION_STYLE =
    R"(
        {
            "ConnectionStyle": {
                "ConstructionColor": "gray",
                "NormalColor": "black",
                "SelectedColor": "gray",
                "SelectedHaloColor": "deepskyblue",
                "HoveredColor": "deepskyblue",

                "LineWidth": 2.0,
                "ConstructionLineWidth": 2.0,
                "PointDiameter": 10.0,

                "UseDataDefinedColors": true
            }
        }
        )";
constexpr ConstLatin1String GRAPHICS_VIEW_STYLE =
    R"(
        {
            "GraphicsViewStyle": {
                "BackgroundColor": "white",
                "FineGridColor": "lightGray",
                "CoarseGridColor": "gray"
            }
        }
    )";
constexpr ConstLatin1String NODE_STYLE =
    R"(
        {
            "NodeStyle": {
                "NormalBoundaryColor": "black",
                "SelectedBoundaryColor": [255, 165, 0],
                "GradientColor0": "white",
                "GradientColor1": "lightGray",
                "GradientColor2": [130, 130, 130],
                "GradientColor3": [90, 90, 90],
                "ShadowColor": [20, 20, 20],
                "FontColor" : "black",
                "FontColorFaded" : [100, 100, 100],
                "ConnectionPointColor": [169, 169, 169],
                "FilledConnectionPointColor": "yellow",
                "ErrorColor": "red",
                "WarningColor": [128, 128, 0],

                "PenWidth": 1.0,
                "HoveredPenWidth": 1.5,

                "ConnectionPointDiameter": 5.0,

                "Opacity": 0.8
            }
        }
    )";

constexpr uint MINUTE_MSECS = 60000;

constexpr ConstLatin1String DATA_PORT_ID = "DataNode";
constexpr ConstLatin1String FUNCTION_PORT_ID = "FunctionNode";

inline bool isFunctionNode(QtNodes::NodeDataType node)
{
    return node.id == constants::FUNCTION_PORT_ID;
}
inline bool isDataNode(QtNodes::NodeDataType node)
{
    return node.id == constants::DATA_PORT_ID;
}

// Define a named constant for the red color used in function port styling
constexpr QColor FUNCTION_PORT_COLOR = QColor(255, 0, 0); // RGB

constexpr uint MAX_DATA_INPUT_PORTS = 20;
constexpr uint MAX_DATA_OUTPUT_PORTS = 20;

namespace kedro {

// relative paths from kedro project dir
constexpr ConstLatin1String CONF_PATH = "conf/base/";
// %1 is the kedro project name
constexpr ConstLatin1String SOURCE_PATH = "src/%1/";
constexpr ConstLatin1String RAW_DATA_PATH = "data/01_raw/";
constexpr ConstLatin1String MODELS_PATH = "data/06_models/";
constexpr ConstLatin1String REPORTING_PATH = "data/08_reporting/";

// templates for gnerating files
constexpr ConstLatin1String CATALOG_YML_ENTRY =
    R"(%1:
  type: %2
  filepath: %3
  )";

// %1 is the list of all pipeline objects
constexpr ConstLatin1String PIPELINE_PY =
    R"(
from kedro.pipeline import Pipeline, node, pipeline
from kedro_umbrella import coder, processor, trainer
from kedro_umbrella.library import *
from .nodes import *

def create_pipeline(**kwargs) -> Pipeline:
    return pipeline(
        [
%1
        ]
    )
    )";
} // namespace kedro

// error messages for warning pop ups
const QString TYPE_MISMATCH = "Failed to connect '%1'. The expected input is of "
                              "type '%2'.\nPlease verify your "
                              "connections or use 'Override' to indicate that "
                              "these two data types are the same.";
const QString SINGULAR_SIGNATURE
    = "The expected function signature is singular. Please check the connection.";

const QString WARN_MANUAL_OVERRIDE = "The tag you entered already "
                                     "exists. Do you want to "
                                     "override the existing tag?";
} // namespace constants
