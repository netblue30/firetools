APPS = firetools firemgr debmenu firejail-ui
.PHONY: apps $(APPS)
apps: $(APPS)
$(APPS):
	$(MAKE) -C $@

clean:;
	for dir in $(APPS); do \
		$(MAKE) -C $$dir clean; \
	done
