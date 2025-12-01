# Maintainer: luengor <luengor at lueng dot org>
pkgname="hyprgrab"
pkgver=e6442d0
pkgrel=1
pkgdesc="A screenshot and screen recording tool for Hyprland"
arch=('x86_64')
url="https://github.com/Luengor/hyprgrab"
license=('GPL-3.0-or-later')
depends=('hyprland' 'grim' 'slurp' 'wl-clipboard' 'libnotify' 'jq' 'glibc' 'gcc-libs')
makedepends=('git' 'make' 'clang')
provides=('hyprgrab')
conflicts=('hyprgrab')
source=("hyprgrab::git+https://github.com/Luengor/hyprgrab.git")
sha256sums=('SKIP')

pkgver() {
  cd "$srcdir/hyprgrab"
  git describe --tags --abbrev=0 2>/dev/null || git rev-parse --short HEAD
}

build() {
    cd "$srcdir/hyprgrab" || exit
    make
}

package() {
    cd "$srcdir/hyprgrab" || exit
    mkdir -p "$pkgdir/usr/bin"
    make INSTALL_DIR="$pkgdir/usr/bin" install
}
