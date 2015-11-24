ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src test
dist_noinst_DATA = LICENSE README.rst
distdir = @PACKAGE_NAME@-@VERSION@.src

## Custom targets ##

PROJECT_DIR = $(CURDIR)
PROJECT_REPORT_DIR = $(PROJECT_DIR)/report
PROJECT_SITE_DIR = $(PROJECT_DIR)/site
CPPUNIT_REPORT_DIR = $(PROJECT_REPORT_DIR)/test
CPPCHECK_REPORT_DIR = $(PROJECT_REPORT_DIR)/cppcheck
COVERAGE_REPORT_DIR = $(PROJECT_REPORT_DIR)/coverage
COVERAGE_LCOV_DIR = $(COVERAGE_REPORT_DIR)/lcov-report

CPPCHECK_FLAGS  = -I/usr/include $(CPPFLAGS)
CPPCHECK_FLAGS += --suppress='*:/usr/include/*.h'
CPPCHECK_FLAGS += --suppress='*:/usr/include/curl/*.h'
CPPCHECK_FLAGS += --enable=all --max-configs=100

XSLTPROC_FLAGS  = $(PROJECT_DIR)/tools/build/files/cppunit2xunit.xsl

# function to get relative path
relpath = $(subst $(CURDIR)/,,$(1))

# function to call cppcheck with args $1 and filter output to file/descriptor $2
cppcheck = { cppcheck $(CPPCHECK_FLAGS) $(1) 2>&1 1>&3 | fgrep -v 'Unmatched suppression' 1>$(2); } 3>&1

# remove generated reports
clean-local:
	@rm -rf $(PROJECT_REPORT_DIR)
	@rm -rf $(PROJECT_SITE_DIR)

# check coding style
lint:
	$(call cppcheck,src,&2) || true

# generate coding style report
lint-report: REPORT_DIR = $(call relpath,$(CPPCHECK_REPORT_DIR))
lint-report:
	@mkdir -p $(CPPCHECK_REPORT_DIR)
	$(call cppcheck,--xml src,$(CPPCHECK_REPORT_DIR)/cppcheck-result.xml)
	@echo "Generated \`$(REPORT_DIR)/cppcheck-result.xml'"

# synonym for standard 'check' target
test: check

# generate xunit test reports
test-report: REPORT_DIR = $(call relpath,$(CPPUNIT_REPORT_DIR))
test-report:
	@$(MAKE) check
	@rm -rf $(CPPUNIT_REPORT_DIR)
	@mkdir -p $(CPPUNIT_REPORT_DIR)
	@cd test; for i in *-cppunit-results.xml; do \
		xsltproc $(XSLTPROC_FLAGS) $$i \
		> $(CPPUNIT_REPORT_DIR)/TEST-xunit-$${i%-cppunit-results.xml}.xml \
		&& echo "Generated \`$(REPORT_DIR)/TEST-xunit-$${i%-cppunit-results.xml}.xml'"; \
	done

if !GCOV_ENABLED

coverage coverage-report:
	@printf "Please run \`./configure --enable-gcov'\n\n" 1>&2

else GCOV_ENABLED

# check coverage
coverage:
	@echo "Running tests..."
	@$(MAKE) check >/dev/null
	cd src/.libs; gcovr --root=. --print-summary

# generate coverage report
coverage-report: REPORT_DIR = $(call relpath,$(COVERAGE_REPORT_DIR))
coverage-report: LCOV_DIR = $(call relpath,$(COVERAGE_LCOV_DIR))
coverage-report:
	@echo "Running tests..."
	@$(MAKE) check >/dev/null
	@rm -rf $(COVERAGE_LCOV_DIR)
	@mkdir -p $(COVERAGE_LCOV_DIR)
	@mkdir -p $(COVERAGE_REPORT_DIR)
	lcov -d src/.libs --capture --output-file $(COVERAGE_REPORT_DIR)/lcov.info
	lcov -r $(COVERAGE_REPORT_DIR)/lcov.info "/usr/include/*" -o $(COVERAGE_REPORT_DIR)/lcov.info
	genhtml -o $(COVERAGE_LCOV_DIR) $(COVERAGE_REPORT_DIR)/lcov.info
	@sed -i s:"$(CURDIR)":".":g $(COVERAGE_REPORT_DIR)/lcov.info
	cd src/.libs; gcovr --root=. -x -o $(COVERAGE_REPORT_DIR)/cobertura-coverage.xml
	@sed -i s#filename=\"#filename=\"src/#g $(COVERAGE_REPORT_DIR)/cobertura-coverage.xml
	@for i in $(REPORT_DIR)/*; do echo "Generated \`$$i'"; done
	@if [ -d "$(LCOV_DIR)" ]; then  echo "Generated \`$(LCOV_DIR)'"; fi

endif

# generate documentation
doc:
	@mkdir -p $(PROJECT_SITE_DIR)
	doxygen
