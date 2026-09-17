#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PACKAGE_NAME=graceful-desktop-environment
VERSION=${VERSION:-0.1.0}
ARCH=${ARCH:-$(dpkg --print-architecture)}
BUILD_DIR=${BUILD_DIR:-"$ROOT_DIR/build/package/build"}
PKG_ROOT=${PKG_ROOT:-"$ROOT_DIR/build/package/root"}
DEB_DIR=${DEB_DIR:-"$ROOT_DIR/build/deb"}
CONTROL_TEMPLATE="$ROOT_DIR/packaging/debian-control.in"
LIGHTDM_CONFIG="$ROOT_DIR/packaging/lightdm/50-graceful.conf"

rm -rf "$PKG_ROOT" "$DEB_DIR"
mkdir -p "$BUILD_DIR" "$PKG_ROOT" "$DEB_DIR"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$BUILD_DIR"
ctest --test-dir "$BUILD_DIR" --output-on-failure
DESTDIR="$PKG_ROOT" cmake --install "$BUILD_DIR"

install -Dm644 "$LIGHTDM_CONFIG" "$PKG_ROOT/etc/lightdm/lightdm.conf.d/50-graceful.conf"

mkdir -p "$PKG_ROOT/DEBIAN"
installed_size=$(du -sk "$PKG_ROOT" | awk '{print $1}')
sed \
    -e "s/@VERSION@/$VERSION/g" \
    -e "s/@ARCH@/$ARCH/g" \
    -e "s/@INSTALLED_SIZE@/$installed_size/g" \
    "$CONTROL_TEMPLATE" > "$PKG_ROOT/DEBIAN/control"
printf '%s\n' "/etc/lightdm/lightdm.conf.d/50-graceful.conf" > "$PKG_ROOT/DEBIAN/conffiles"
cat > "$PKG_ROOT/DEBIAN/postinst" <<'POSTINST'
#!/bin/sh
set -e

rm -f /usr/share/xsessions/graceful.desktop

exit 0
POSTINST
chmod 755 "$PKG_ROOT/DEBIAN/postinst"

dpkg-deb --build --root-owner-group "$PKG_ROOT" "$DEB_DIR/${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"
echo "$DEB_DIR/${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"
