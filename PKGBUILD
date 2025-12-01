# Maintainer: luengor <luengor at lueng dot org>
pkgname="hyprgrab-git"
pkgver=r35.7c86178
pkgrel=1
pkgdesc="A screenshot and screen recording tool for Hyprland"
arch=('x86_64')
url="https://github.com/Luengor/hyprgrab"
license=('0BSD')
depends=('hyprland' 'grim' 'slurp' 'wl-clipboard' 'libnotify' 'jq' 'glibc' 'gcc-libs' 'wl-screenrec')
makedepends=('git' 'make' 'clang')
provides=("hyprgrab")
conflicts=("hyprgrab")
source=("hyprgrab-$pkgver::git+https://github.com/Luengor/hyprgrab.git")
sha256sums=('SKIP')

pkgver() {
  cd "$srcdir/hyprgrab-$pkgver" || exit
  ( set -o pipefail
    git describe --long --abbrev=7 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short=7 HEAD)"
  )
}

build() {
  cd "$srcdir/hyprgrab-$pkgver" || exit
  make
}

package() {
  cd "$srcdir/hyprgrab-$pkgver" || exit
  mkdir -p "$pkgdir/usr/bin"
  make INSTALL_DIR="$pkgdir/usr/bin" install
}
