all: glide2x.dll glide2x.ovl shasum

glide2x.ovl: ovl.xxd
	@echo "  XXD $@"
	@xxd -r $< $@

glide2x.dll: dll.xxd
	@echo "  XXD $@"
	@xxd -r $< $@

shasum: shasum.sig
	@shasum glide2x.dll glide2x.ovl | sed "s/.\(glide2x\)/\1/" | diff - $<

clean:
	@rm -f *.ovl *.dll
