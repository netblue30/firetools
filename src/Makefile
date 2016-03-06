APPS = firetools firemgr
.PHONY: apps $(APPS)
apps: $(APPS)
$(APPS):
	$(MAKE) -C $@

clean:;
	for dir in $(APPS); do \
		$(MAKE) -C $$dir clean; \
	done
