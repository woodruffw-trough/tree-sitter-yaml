TS ?= tree-sitter
PYTHON ?= python3

.PHONY: generate test

generate:
	$(TS) generate --js-runtime native --abi 15
	set -e; for schema in core json legacy; do \
		(cd schema/$$schema && $(TS) generate --js-runtime native --abi 14); \
		$(PYTHON) schema/update-schema.py $$schema; \
	done

test:
	cargo test --locked
	$(TS) test
