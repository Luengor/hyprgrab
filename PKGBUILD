# Maintainer: luengor <luengor at lueng dot org>
_basename='hyprgrab'
pkgname="$_basename-git"
pkgver=r35.7c86178
pkgrel=1
pkgdesc="A screenshot and screen recording tool for Hyprland"
arch=('x86_64')
url="https://github.com/Luengor/hyprgrab"
license=('GPL-3.0-or-later')
depends=('hyprland' 'grim' 'slurp' 'wl-clipboard' 'libnotify' 'jq' 'glibc' 'gcc-libs' 'wl-screenrec')
makedepends=('git' 'make' 'clang')
provides=("$_basename")
conflicts=("$_basename")
source=("$_basename::git+https://github.com/Luengor/hyprgrab.git")
sha256sums=('SKIP')

pkgver() {
  cd "$srcdir/$_basename" || exit
  ( set -o pipefail
    git describe --long --abbrev=7 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short=7 HEAD)"
  )
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
