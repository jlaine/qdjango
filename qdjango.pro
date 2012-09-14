include(qdjango.pri)

TEMPLATE = subdirs

SUBDIRS = src

android {
} else {
    SUBDIRS += tests examples
    INSTALLS += htmldocs
}

CONFIG += ordered

# Documentation generation
docs.commands = cd doc/ && doxygen

# Source distribution
QDJANGO_ARCHIVE = qdjango-$$QDJANGO_VERSION
dist.commands = \
    $(DEL_FILE) -r $$QDJANGO_ARCHIVE && \
    $(MKDIR) $$QDJANGO_ARCHIVE && \
    git archive master | tar -x -C $$QDJANGO_ARCHIVE && \
    $(COPY_DIR) doc/html $$QDJANGO_ARCHIVE/doc && \
    tar czf $${QDJANGO_ARCHIVE}.tar.gz $$QDJANGO_ARCHIVE && \
    $(DEL_FILE) -r $$QDJANGO_ARCHIVE
dist.depends = docs

# Install rules
htmldocs.files = doc/html
htmldocs.path = $$PREFIX/share/doc/qdjango
htmldocs.CONFIG += no_check_exist directory

QMAKE_EXTRA_TARGETS += dist docs
