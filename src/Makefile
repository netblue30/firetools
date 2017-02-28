APPS = firetools firejail-ui fstats fmgr
.PHONY: apps $(APPS)
apps: $(APPS)
$(APPS):
	$(MAKE) -C $@

clean:;
	for dir in $(APPS); do \
		$(MAKE) -C $$dir clean; \
	done

distclean:;
	for dir in $(APPS); do \
		$(MAKE) -C $$dir distclean; \
	done
