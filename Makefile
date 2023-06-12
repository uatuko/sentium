BUILDDIR = .build
BINARY   = $(BUILDDIR)/bin/gatekeeper

PROTODIR  = proto
SOURCEDIR = src

SOURCES  := $(shell find $(PROTODIR) -type f -name '*.proto')
SOURCES  += $(shell find $(SOURCEDIR) -type f -name '*.h' -o -name '*.cpp')


.PHONY: $(BINARY) clean
.SILENT: lint

all: $(BINARY)

$(BINARY): $(BUILDDIR)
	find $(BUILDDIR)/$(SOURCEDIR) -name "*.gcda" -exec rm {} +
	cmake --build $(BUILDDIR)

$(BUILDDIR):
	cmake -B $(BUILDDIR) -G Ninja -DGATEKEEPER_ENABLE_COVERAGE=ON -DGATEKEEPER_ENABLE_TESTING=ON

clean:
	cmake --build $(BUILDDIR) --target clean

lint:
ifeq (, $(shell which clang-format))
	echo '\033[1;41m WARN \033[0m clang-format not found, not linting files';
else
	clang-format -style=file --dry-run $(SOURCES)
endif

lint\:ci:
	clang-format -style=file --dry-run -Werror $(SOURCES)

lint\:fix:
	clang-format -style=file -i $(SOURCES)

run: $(BINARY)
	$(BINARY)

test: $(BINARY)
	ctest --test-dir .build --output-on-failure

