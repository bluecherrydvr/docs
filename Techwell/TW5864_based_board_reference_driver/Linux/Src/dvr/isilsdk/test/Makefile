#ifneq ($(GLABALCOMPILE),yes)
#	override TOPDIR := ./../
#	sinclude	$(TOPDIR)rules.make	
#endif

export

SUBDIRS = audio_test h264_main_test h264_sub_test mjpeg_test
.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	@echo -e "Building  testing  example .\n\t"
	$(MAKE) -C $@

.PHONY: clean
clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean;done
