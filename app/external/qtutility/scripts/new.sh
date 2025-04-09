# this script will generate .cpp and .hpp files for when you have split src/ and include/ dirs
# directories is assumed to be fixed to the following structure:
# root/
# ├── include/
# │   └── (.hpp will be generated here)
# ├── scripts/
# │   └── new.sh
# └── src/
#     └── (.cpp will be generated here)

#!/bin/bash

# Assuming operations based on the project root directory
PROJECT_ROOT=$(realpath $(dirname "$0")/..)

# Check if the argument is provided
if [ -z "$1" ]; then
  echo "Usage: $0 path/to/filename [-qobject | -qwidget | none]"
  exit 1
fi

underscore_to_upper_camel() {
  echo "$1" | tr '_' '\n' | awk '{print toupper(substr($0,1,1)) tolower(substr($0,2))}' | tr -d '\n'
}

# Extract the directory and filename from the argument
DIR=$(dirname "$1")
FILENAME=$(basename "$1")
CLASSNAME=$(underscore_to_upper_camel "$FILENAME")

# Check for -qt option
QT_OPTION=""
if [ "$2" == "-qobject" ]; then
  QT_OPTION="qobject"
elif [ "$2" == "-qwidget" ]; then
  QT_OPTION="qwidget"
else
  QT_OPTION=""
fi

# Create the directories if they do not exist
mkdir -p "$PROJECT_ROOT/src/$DIR"
mkdir -p "$PROJECT_ROOT/include/$DIR"

# Function to generate the .cpp file content
generate_cpp_content() {
  if [ "$DIR" == "." ]; then
    echo "#include \"$FILENAME.hpp\"\n\n$CLASSNAME::$CLASSNAME()\n{\n}"
  else
    echo "#include \"$DIR/$FILENAME.hpp\"\n\n$CLASSNAME::$CLASSNAME()\n{\n}"
  fi
}

# Function to generate the .hpp file content
generate_hpp_content() {
  local content="#pragma once\n"
  case "$QT_OPTION" in
  "qobject")
    content+="\n#include <QObject>\n\nclass $CLASSNAME : public QObject\n{\n    Q_OBJECT"
    ;;
  "qwidget")
    content+="\n#include <QWidget>\n\nclass $CLASSNAME : public QWidget\n{\n    Q_OBJECT"
    ;;
  "")
    content+="\nclass $CLASSNAME\n{"
    ;;
  esac
  content+="\npublic:\n    $CLASSNAME();\n};\n"
  echo "$content"
}

# Generate the .cpp file
cpp_content=$(generate_cpp_content)
echo "$cpp_content" >"$PROJECT_ROOT/src/$DIR/$FILENAME.cpp"

# Generate the .hpp file
hpp_content=$(generate_hpp_content)
echo "$hpp_content" >"$PROJECT_ROOT/include/$DIR/$FILENAME.hpp"

echo "$FILENAME.cpp and $FILENAME.hpp created in $PROJECT_ROOT/src/$DIR and $PROJECT_ROOT/include/$DIR"
