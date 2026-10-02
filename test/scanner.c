#include "../src/scanner.c"

static void test_serialization_bounds(void) {
    Scanner scanner = {0};
    Scanner restored = {0};
    char *buffer = malloc(TREE_SITTER_SERIALIZATION_BUFFER_SIZE);
    assert(buffer != NULL);
    deserialize(&scanner, NULL, 0);

    unsigned header_size = serialize(&scanner, buffer);
    unsigned entry_size = sizeof(uint8_t) + sizeof(int16_t);
    unsigned capacity = (TREE_SITTER_SERIALIZATION_BUFFER_SIZE - header_size) / entry_size;
    assert(capacity >= 253);
    const int16_t types[] = {IND_MAP, IND_SEQ, IND_STR};

    for (unsigned count = 1; count <= capacity + 2; count++) {
        push_ind(&scanner, types[(count - 1) % 3], (int16_t)count);
        unsigned length = serialize(&scanner, buffer);
        unsigned saved = count < capacity ? count : capacity;
        assert(length <= TREE_SITTER_SERIALIZATION_BUFFER_SIZE);
        assert(length == header_size + saved * entry_size);

        deserialize(&restored, buffer, length);
        assert(restored.ind_typ_stk.size == saved + 1);
        assert(restored.ind_len_stk.size == saved + 1);
        for (unsigned i = 1; i <= saved; i++) {
            assert(restored.ind_typ_stk.contents[i] == types[(i - 1) % 3]);
            assert(restored.ind_len_stk.contents[i] == (int16_t)i);
        }
    }

    array_delete(&scanner.ind_typ_stk);
    array_delete(&scanner.ind_len_stk);
    array_delete(&restored.ind_typ_stk);
    array_delete(&restored.ind_len_stk);
    free(buffer);
}

static void test_row_serialization(void) {
    Scanner scanner = {0};
    Scanner restored = {0};
    _Alignas(uint32_t) char buffer[TREE_SITTER_SERIALIZATION_BUFFER_SIZE + 1];
    const uint32_t rows[] = {32767, 32768, 65536, UINT32_MAX - 1};
    deserialize(&scanner, NULL, 0);

    for (unsigned i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        scanner.row = rows[i];
        scanner.blk_imp_row = rows[i] - 1;
        scanner.col = 3;
        scanner.blk_imp_col = 1;
        // Offset the aligned backing array to exercise an unaligned byte buffer.
        unsigned length = serialize(&scanner, buffer + 1);
        deserialize(&restored, buffer + 1, length);
        assert(restored.row == scanner.row);
        assert(restored.blk_imp_row == scanner.blk_imp_row);
        assert(restored.col == scanner.col);
        assert(restored.blk_imp_col == scanner.blk_imp_col);
    }

    array_delete(&scanner.ind_typ_stk);
    array_delete(&scanner.ind_len_stk);
    array_delete(&restored.ind_typ_stk);
    array_delete(&restored.ind_len_stk);
}

int main(void) {
    test_serialization_bounds();
    test_row_serialization();
    return 0;
}
