# Maintainer: Siddharth Karanam <sid9.karanam@gmail.com>
pkgname=litemus
pkgver=1.0
pkgrel=1
pkgdesc="Litemus: A light music player using ncurses and SFML libraries"
arch=('x86_64')
url="http://github.com/nots1dd/Litemus"
license=('GPL')
depends=('sfml' 'nlohmann-json' 'ncurses')
makedepends=('cmake' 'make')
source=("git+https://github.com/nots1dd/Litemus.git#branch=main")
sha256sums=('SKIP')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    cmake -Bbuild -H. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    make -C build
}

package() {
    cd "$srcdir/$pkgname-$pkgver/build"
    make DESTDIR="$pkgdir/" install
}

