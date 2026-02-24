#!/usr/bin/env bash
set -e

APP_NAME="DescartesBuilder"
APPDIR="$(pwd)/${APP_NAME}.AppDir"

mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/plugins/platforms"

echo "→ Copying application binary"
cp app/build/bin/${APP_NAME} "$APPDIR/usr/bin/"

echo "→ Copying project libraries"
cp app/build/libDescartesBuilder_lib.so "$APPDIR/usr/lib/"
cp app/build/lib/libQtNodes.so "$APPDIR/usr/lib/"
cp app/build/external/qtutility/libQtUtility.so "$APPDIR/usr/lib/"
cp app/build/external/quazip/quazip/libquazip1-qt6.so.1.5.0 "$APPDIR/usr/lib/"

echo "→ Copying Qt libraries"
cp "$Qt6_DIR/lib/libQt6"*.so* "$APPDIR/usr/lib/"
cp "$Qt6_DIR/lib/libicu"*.so* "$APPDIR/usr/lib/"

echo "→ Copying Qt platform plugins (xcb)"
cp "$Qt6_DIR/plugins/platforms/libq"*.so \
   "$APPDIR/usr/plugins/platforms/"

echo "→ Writing qt.conf"
cat > "$APPDIR/usr/bin/qt.conf" <<EOF
[Paths]
Plugins = ../plugins
Libraries = ../lib
EOF

echo "→ Writing AppRun"
cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "$0")")"

export PATH="$HERE/usr/python/bin:$PATH"
export PYTHONHOME="$HERE/usr/python"
export PYTHONPATH="$HERE/usr/python/lib/python3.10/site-packages:$PYTHONPATH"

export LD_LIBRARY_PATH="$HERE/usr/lib:$LD_LIBRARY_PATH"

export QT_PLUGIN_PATH="$HERE/usr/plugins"
export QT_QPA_PLATFORM=xcb
export QT_XCB_CURSOR=1
export QT_LOGGING_RULES="qt.qpa.*=false"

exec "$HERE/usr/bin/DescartesBuilder" "$@"
EOF

chmod +x "$APPDIR/AppRun"

echo "→ Desktop entry + icon"
cat > "$APPDIR/${APP_NAME}.desktop" <<EOF
[Desktop Entry]
Name=${APP_NAME}
Exec=${APP_NAME}
Icon=${APP_NAME}
Type=Application
Categories=Utility;
EOF

cp app/resources/tool_logo.png "$APPDIR/${APP_NAME}.png"