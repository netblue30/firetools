APPS = firelauncher fmgr firejail-ui firetools
.PHONY: apps $(APPS)
apps: $(APPS)
$(APPS):
	$(MAKE) -C $@

clean:;
	for dir in $(APPS); do \
		$(MAKE) -C $$dir clean; \
	done
