# Maintainer: luengor <luengor at lueng dot org>
_basename='hyprgrab'
pkgname="$_basename-git"
pkgver=7c86178
pkgrel=1
pkgdesc="A screenshot and screen recording tool for Hyprland"
arch=('x86_64')
url="https://github.com/Luengor/hyprgrab"
license=('GPL-3.0-or-later')
depends=('hyprland' 'grim' 'slurp' 'wl-clipboard' 'libnotify' 'jq' 'glibc' 'gcc-libs')
makedepends=('git' 'make' 'clang')
provides=("$_basename")
conflicts=("$_basename")
source=("$_basename::git+https://github.com/Luengor/hyprgrab.git")
sha256sums=('SKIP')

pkgver() {
  cd "$srcdir/$_basename" || exit
  git describe --tags --abbrev=0 2>/dev/null || git rev-parse --short HEAD
}

build() {
    cd "$srcdir/$_basename" || exit
    make
}

package() {
    cd "$srcdir/$_basename" || exit
    mkdir -p "$pkgdir/usr/bin"
    make INSTALL_DIR="$pkgdir/usr/bin" install
}
