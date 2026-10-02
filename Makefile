TS ?= tree-sitter
PYTHON ?= python3

.PHONY: generate test test-scanner

generate:
	$(TS) generate --js-runtime native --abi 15
	set -e; for schema in core json legacy; do \
		(cd schema/$$schema && $(TS) generate --js-runtime native --abi 14); \
		$(PYTHON) schema/update-schema.py $$schema; \
	done

test: test-scanner
	cargo test --locked
	$(TS) test

test-scanner:
	mkdir -p target
	$(CC) $(CFLAGS) -std=c11 -Isrc test/scanner.c -o target/scanner-test
	./target/scanner-test
