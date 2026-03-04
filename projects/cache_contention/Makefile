clean:
	rm -rf ./builddir/

build:
	 meson setup builddir --cross-file=../../cross/rv64imac.ini --buildtype debug
	 ninja -C ./builddir/

run: build
	ninja -C ./builddir/ qemu-run

plugin-run: build
	ninja -C ./builddir/ qemu-run-plugin

